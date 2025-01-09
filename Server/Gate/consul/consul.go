package consul

import (
	"Gate/settings"
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
	ip = service[0].Service.Address
	port = service[0].Service.Port
	return
}
