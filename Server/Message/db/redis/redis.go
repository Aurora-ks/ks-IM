package redis

import (
	"Message/settings"
	"context"
	"errors"
	"fmt"
	"github.com/redis/go-redis/v9"
)

const (
	kUserOnlineList = "user:online" // set [UserID]
	kUserInServer   = "server:user" // hash [UserID]-[MachineID]
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

// UserOnline 用户上线
func UserOnline(userId string) (err error) {
	err = rds.SAdd(ctx, kUserOnlineList, userId).Err()
	if err != nil {
		return
	}
	err = rds.HSet(context.Background(), kUserInServer, userId, settings.Conf.ID).Err()
	return
}

// UserOffline 用户下线
func UserOffline(userId string) (err error) {
	err = rds.SRem(ctx, kUserOnlineList, userId).Err()
	if err != nil {
		return
	}
	err = rds.HDel(ctx, kUserInServer, userId).Err()
	return
}

// GetUserServer 获取用户所在服务器
func GetUserServer(userId string) (serverId int64, err error) {
	serverId, err = rds.HGet(ctx, kUserInServer, userId).Int64()
	return
}

// GetUserOnlineList 获取在线用户列表
func GetUserOnlineList() (userList []string, err error) {
	userList, err = rds.SMembers(ctx, kUserOnlineList).Result()
	return
}

// IsUserOnline 查询用户是否在线
func IsUserOnline(userId string) (isOnline bool, err error) {
	isOnline = rds.SIsMember(ctx, kUserOnlineList, userId).Val()
	return
}

// GetUserMachineID 获取用户所在服务器ID
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
