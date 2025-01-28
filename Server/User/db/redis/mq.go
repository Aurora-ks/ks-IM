package redis

import (
	"User/logic"

	"github.com/redis/go-redis/v9"
)

const KP_MQ = "mq:"

func WriteToMQ(msg *logic.MQMsg) error {
	mq := KP_MQ + msg.ReceivedMachID
	err := rds.XAdd(ctx, &redis.XAddArgs{
		Stream: mq,
		ID:     "*",
		Values: msg.ToMap(),
	}).Err()
	return err
}
