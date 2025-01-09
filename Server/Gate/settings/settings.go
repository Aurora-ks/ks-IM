package settings

import "github.com/spf13/viper"

type ServerConfig struct {
	Name string `mapstructure:"name"`
	ID   string `mapstructure:"id"`
	Port int    `mapstructure:"port"`
}

type ConsulConfig struct {
	Host string `mapstructure:"host"`
	Port int    `mapstructure:"port"`
}

type Config struct {
	*ServerConfig `mapstructure:"server"`
	*ConsulConfig `mapstructure:"consul"`
}

var Conf = new(Config)

func LoadConf() (err error) {
	viper.SetConfigFile("config.yaml")
	viper.AddConfigPath(".")
	err = viper.ReadInConfig()
	if err != nil {
		return
	}
	err = viper.Unmarshal(Conf)
	return
}
