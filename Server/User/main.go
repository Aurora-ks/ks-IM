package main

import (
	"User/consul"
	"User/db/mysql"
	"User/db/redis"
	"User/log"
	"User/router"
	"User/settings"
	"context"
	"errors"
	"fmt"
	"net"
	"net/http"
	"os"
	"os/signal"
	"syscall"
	"time"

	"github.com/hashicorp/consul/api"
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

	if err := RegisterService(); err != nil {
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

	if err := redis.Init(); err != nil {
		log.L().Error("Redis Init", log.Error(err))
		return
	}

	r := router.SetUp()
	srv := &http.Server{
		Addr:    fmt.Sprintf(":%d", settings.Conf.ServerConfig.Port),
		Handler: r,
	}
	go func() {
		log.L().Info("Server Start", log.Int("port", settings.Conf.ServerConfig.Port))
		if err := srv.ListenAndServe(); err != nil && !errors.Is(err, http.ErrServerClosed) {
			log.L().Error("Server Listen", log.Error(err))
			return
		}
	}()

	quit := make(chan os.Signal, 1)
	signal.Notify(quit, syscall.SIGINT, syscall.SIGTERM)
	<-quit
	log.L().Info("Shutdown Server")

	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()

	if err := srv.Shutdown(ctx); err != nil {
		log.L().Error("Server Shutdown", log.Error(err))
		return
	}
	log.L().Info("Server Exiting")
}

func GetIpv4() ([]string, error) {
	var ips []string
	ifaces, err := net.Interfaces()
	if err != nil {
		return nil, err
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
			return nil, err
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
	return ips, nil
}

func RegisterService() (err error) {
	ips, err := GetIpv4()
	if err != nil {
		return
	}
	service := &api.AgentServiceRegistration{
		ID:      settings.Conf.ServerConfig.ID,
		Name:    settings.Conf.ServerConfig.Name,
		Address: ips[0],
		Port:    settings.Conf.ServerConfig.Port,
		Tags:    []string{"User"},
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
