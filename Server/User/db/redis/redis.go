package redis

import (
	"User/settings"
	"context"
	"errors"
	"fmt"
	"time"

	"github.com/redis/go-redis/v9"
)

const (
	PFX_UserVerfyCode = "user:VerifyCode:"
	kUserInServer     = "server:user" // hash [UserID]-[MachineID]
)

var rds *redis.Client
var ctx = context.Background()

func Init() (err error) {
	rds = redis.NewClient(&redis.Options{
		Addr:     fmt.Sprintf("%s:%d", settings.Conf.RedisConfig.Host, settings.Conf.RedisConfig.Port),
		Password: settings.Conf.RedisConfig.Password,
		DB:       settings.Conf.RedisConfig.DB,
	})
	_, err = rds.Ping(ctx).Result()
	return
}

func Close() {
	_ = rds.Close()
}

func GetUserVerifyCode(email string) (code string) {
	ctx1, cancel := context.WithTimeout(ctx, 500*time.Microsecond)
	defer cancel()
	code = rds.Get(ctx1, PFX_UserVerfyCode+email).Val()
	return
}

func SetUserVerifyCode(email, code string) (err error) {
	ctx1, cancel := context.WithTimeout(ctx, 500*time.Microsecond)
	defer cancel()
	err = rds.Set(ctx1, PFX_UserVerfyCode+email, code, 3*time.Minute).Err()
	return
}
func GetUserMachineID(userId string) (isOnline bool, machineID string, err error) {
	machineID, err = rds.HGet(ctx, kUserInServer, userId).Result()
	// 用户不在线
	if errors.Is(err, redis.Nil) {
		isOnline = false
		err = nil
	} else {
		isOnline = true
	}
	return
}
