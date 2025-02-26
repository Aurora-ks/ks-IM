package redis

import (
	"github.com/redis/go-redis/v9"
)

const KP_MQ = "mq:"

type MQMsg struct {
	SenderMachID   string `json:"sender_machine_id"`
	ReceivedMachID string `json:"received_machine_id"`
	UserID         uint64 `json:"user_id"`
	Cmd            uint32 `json:"cmd"`
	MsgType        uint32 `json:"msg_type"`
	Data           []byte `json:"data"`
}

func (m *MQMsg) ToMap() map[string]any {
	return map[string]any{
		"sender_machine_id":   m.SenderMachID,
		"received_machine_id": m.ReceivedMachID,
		"user_id":             m.UserID,
		"cmd":                 m.Cmd,
		"msg_type":            m.MsgType,
		"data":                m.Data,
	}
}

func (m *MQMsg) FromMap(mq map[string]any) {
	m.SenderMachID = mq["sender_id"].(string)
	m.ReceivedMachID = mq["received_id"].(string)
	m.UserID = mq["user_id"].(uint64)
	m.Cmd = mq["cmd"].(uint32)
	m.MsgType = mq["msg_type"].(uint32)
	m.Data = mq["data"].([]byte)
}

func WriteToMQ(msg *MQMsg) error {
	mq := KP_MQ + msg.ReceivedMachID
	err := rds.XAdd(ctx, &redis.XAddArgs{
		Stream: mq,
		ID:     "*",
		Values: msg.ToMap(),
	}).Err()
	return err
}
