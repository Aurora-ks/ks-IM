package redis

import (
	"User/settings"
	"context"
	"fmt"
	"time"

	"github.com/redis/go-redis/v9"
)

const (
	PFX_UserVerfyCode = "user:VerifyCode:"
)

var rds *redis.Client

func Init() (err error) {
	rds = redis.NewClient(&redis.Options{
		Addr:     fmt.Sprintf("%s:%d", settings.Conf.RedisConfig.Host, settings.Conf.RedisConfig.Port),
		Password: settings.Conf.RedisConfig.Password,
		DB:       settings.Conf.RedisConfig.DB,
	})
	_, err = rds.Ping(context.Background()).Result()
	return
}

func Close() {
	_ = rds.Close()
}

func GetUserVerifyCode(email string) (code string) {
	ctx, cancel := context.WithTimeout(context.Background(), 500*time.Microsecond)
	defer cancel()
	code = rds.Get(ctx, PFX_UserVerfyCode+email).Val()
	return
}

func SetUserVerifyCode(email, code string) (err error) {
	ctx, cancel := context.WithTimeout(context.Background(), 500*time.Microsecond)
	defer cancel()
	err = rds.Set(ctx, PFX_UserVerfyCode+email, code, 3*time.Minute).Err()
	return
}
