package settings

import "github.com/spf13/viper"

type ServerConfig struct {
	Name     string `mapstructure:"name"`
	ID       string `mapstructure:"id"`
	Host     string `mapstructure:"host"`
	Port     int    `mapstructure:"port"`
	FilePath string `mapstructure:"file_save_relative_path"`
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

type EmailConfig struct {
	Host     string `mapstructure:"host"`
	Port     int    `mapstructure:"port"`
	From     string `mapstructure:"from"`
	Password string `mapstructure:"password"`
}

type Config struct {
	*ServerConfig `mapstructure:"server"`
	*ConsulConfig `mapstructure:"consul"`
	*MysqlConfig  `mapstructure:"mysql"`
	*RedisConfig  `mapstructure:"redis"`
	*EmailConfig  `mapstructure:"email"`
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
