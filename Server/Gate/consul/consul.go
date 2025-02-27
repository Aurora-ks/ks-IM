package consul

import (
	"Gate/settings"
	"errors"
	"fmt"
	"github.com/hashicorp/consul/api"
)

var client *api.Client

func Init() (err error) {
	config := api.DefaultConfig()
	config.Address = fmt.Sprintf("%s:%d", settings.Conf.ConsulConfig.Host, settings.Conf.ConsulConfig.Port)
	client, err = api.NewClient(config)
	return
}

func GetServiceByName(name string) (ip string, port int, err error) {
	service, _, err := client.Health().Service(name, "", true, nil)
	if err != nil {
		return
	}
	if len(service) == 0 {
		return "", 0, errors.New("no service found with the given name:" + name)
	}
	ip = service[0].Service.Address
	port = service[0].Service.Port
	return
}
