package settings

import "github.com/spf13/viper"

type ServerConfig struct {
	Name      string `mapstructure:"name"`
	ID        string `mapstructure:"id"`
	MachineID int64  `mapstructure:"machine_id"`
	Port      int    `mapstructure:"port"`
}

type ConsulConfig struct {
	Host string `mapstructure:"host"`
	Port int    `mapstructure:"port"`
}

type MysqlConfig struct {
	Host               string `mapstructure:"host"`
	Port               int    `mapstructure:"port"`
	User               string `mapstructure:"user"`
	Password           string `mapstructure:"password"`
	DB                 string `mapstructure:"db"`
	MaxConnections     int    `mapstructure:"max_connections"`
	MaxIdleConnections int    `mapstructure:"max_idle_connections"`
}

type RedisConfig struct {
	Host     string `mapstructure:"host"`
	Port     int    `mapstructure:"port"`
	Password string `mapstructure:"password"`
	DB       int    `mapstructure:"db"`
}

type Config struct {
	*ServerConfig `mapstructure:"server"`
	*ConsulConfig `mapstructure:"consul"`
	*MysqlConfig  `mapstructure:"mysql"`
	*RedisConfig  `mapstructure:"redis"`
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
