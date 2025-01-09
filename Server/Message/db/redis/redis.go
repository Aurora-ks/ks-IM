package redis

import (
	"Message/settings"
	"context"
	"fmt"
	"github.com/redis/go-redis/v9"
)

const (
	kUserOnlineList = "user:online"    // set
	kUserSeq        = "user:sequence:" // string
	kUserInServer   = "server:user"    // hash
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

func UserOnline(userId string) (err error) {
	err = rds.SAdd(context.Background(), kUserOnlineList, userId).Err()
	if err != nil {
		return
	}
	err = rds.HSet(context.Background(), kUserInServer, userId, settings.Conf.ID).Err()
	return
}

func UserOffline(userId string) (err error) {
	err = rds.SRem(context.Background(), kUserOnlineList, userId).Err()
	if err != nil {
		return
	}
	err = rds.HDel(context.Background(), kUserInServer, userId).Err()
	return
}
