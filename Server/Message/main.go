package main

import (
	"Message/consul"
	"Message/db/mysql"
	"Message/db/redis"
	"Message/log"
	"Message/logic"
	"Message/router"
	"Message/settings"
	"context"
	"errors"
	"fmt"
	"github.com/hashicorp/consul/api"
	"net"
	"net/http"
	"os"
	"os/signal"
	"syscall"
	"time"
)

func main() {
	log.Init()

	if err := settings.LoadConf(); err != nil {
		log.L().Error("Load Config", log.Error(err))
		return
	}

	if err := consul.Init(); err != nil {
		log.L().Error("Consul Connect", log.Error(err))
		return
	}

	if err := registerService(); err != nil {
		log.L().Error("Register Service", log.Error(err))
		return
	}
	defer func() {
		if err := consul.Deregister(settings.Conf.ID); err != nil {
			log.L().Error("Deregister Service", log.Error(err))
		}
	}()

	if err := mysql.Init(); err != nil {
		log.L().Error("Mysql Init", log.Error(err))
		return
	}
	defer mysql.Close()

	if err := redis.Init(); err != nil {
		log.L().Error("Redis Init", log.Error(err))
		return
	}
	defer redis.Close()

	r := router.SetUp()
	srv := &http.Server{
		Addr:    fmt.Sprintf(":%d", settings.Conf.ServerConfig.Port),
		Handler: r,
	}
	go func() {
		if err := srv.ListenAndServe(); err != nil && !errors.Is(err, http.ErrServerClosed) {
			log.L().Error("Server Listen", log.Error(err))
			return
		}
	}()

	c, done := context.WithCancel(context.Background())
	go logic.ListenMQ(c)

	log.L().Info(fmt.Sprintf("[%s] Runing On Port %d", settings.Conf.ServerConfig.ID, settings.Conf.ServerConfig.Port))
	quit := make(chan os.Signal, 1)
	signal.Notify(quit, syscall.SIGINT, syscall.SIGTERM)
	<-quit
	done()
	log.L().Info("Shutdown Server")

	ctx, cancel := context.WithTimeout(context.Background(), 10*time.Second)
	defer cancel()

	if err := srv.Shutdown(ctx); err != nil {
		log.L().Error("Server Shutdown", log.Error(err))
		return
	}
	log.L().Info("Server Exiting")
}

func getIpv4() (ips []string, err error) {
	ifaces, err := net.Interfaces()
	if err != nil {
		return
	}

	for _, iface := range ifaces {
		if iface.Flags&net.FlagUp == 0 {
			continue // 接口未激活
		}
		if iface.Flags&net.FlagLoopback != 0 {
			continue // 排除回环地址
		}

		addrs, err := iface.Addrs()
		if err != nil {
			return
		}

		for _, addr := range addrs {
			var ip net.IP
			switch v := addr.(type) {
			case *net.IPNet:
				ip = v.IP
			case *net.IPAddr:
				ip = v.IP
			}
			if ip == nil || ip.IsLoopback() || ip.To4() == nil {
				continue
			}
			ips = append(ips, ip.String())
		}
	}
	return
}

func registerService() (err error) {
	ips, err := getIpv4()
	if err != nil {
		return
	}
	service := &api.AgentServiceRegistration{
		ID:      settings.Conf.ServerConfig.ID,
		Name:    settings.Conf.ServerConfig.Name,
		Address: ips[0],
		Port:    settings.Conf.ServerConfig.Port,
		Tags:    []string{"Message"},
		Check: &api.AgentServiceCheck{
			HTTP:                           fmt.Sprintf("http://%s:%d/health", ips[0], settings.Conf.ServerConfig.Port),
			Interval:                       "10s",
			Timeout:                        "10s",
			DeregisterCriticalServiceAfter: "30s",
		},
	}
	err = consul.Register(service)
	return
}
