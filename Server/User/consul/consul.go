package consul

import (
	"User/settings"
	"fmt"

	"github.com/hashicorp/consul/api"
)

var client *api.Client

func Init() (err error) {
	cnf := api.DefaultConfig()
	cnf.Address = fmt.Sprintf("%s:%d", settings.Conf.ConsulConfig.Host, settings.Conf.ConsulConfig.Port)
	client, err = api.NewClient(cnf)
	if err != nil {
		return
	}
	return
}

func Register(service *api.AgentServiceRegistration) (err error) {
	err = client.Agent().ServiceRegister(service)
	return
}

func Deregister(id string) error {
	return client.Agent().ServiceDeregister(id)
}
