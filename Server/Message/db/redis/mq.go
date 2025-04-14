package redis

import (
	"Message/settings"
	"github.com/redis/go-redis/v9"
	"time"
)

const KP_MQ = "mq:"

func ReadFromMQ(readCnt int, blockTimeMs time.Duration) ([]redis.XMessage, error) {
	mq := KP_MQ + settings.Conf.ID
	start := "$"
	messages, err := rds.XRead(ctx, &redis.XReadArgs{
		Count:   int64(readCnt),
		Block:   blockTimeMs,
		Streams: []string{mq, start},
	}).Result()
	if err != nil {
		return nil, err
	}
	if len(messages) == 0 {
		return nil, nil
	}
	return messages[0].Messages, nil
}

func WriteToMQ(receivedMachID string, val map[string]any) error {
	mq := KP_MQ + receivedMachID
	err := rds.XAdd(ctx, &redis.XAddArgs{
		Stream: mq,
		ID:     "*",
		Values: val,
	}).Err()
	return err
}
