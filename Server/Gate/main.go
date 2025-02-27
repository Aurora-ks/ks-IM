package main

import (
	"Gate/consul"
	"Gate/log"
	"Gate/router"
	"Gate/settings"
	"context"
	"errors"
	"fmt"
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
		log.L().Error("Init Consul", log.Error(err))
		return
	}

	r := router.SetUp()

	srv := &http.Server{
		Addr:    fmt.Sprintf(":%d", settings.Conf.ServerConfig.Port),
		Handler: r,
	}

	go func() {
		log.L().Info("server start", log.Int("port", settings.Conf.ServerConfig.Port))
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
		log.L().Error("Server Shutdown", err)
		return
	}

	log.L().Info("Server Exited")
}
