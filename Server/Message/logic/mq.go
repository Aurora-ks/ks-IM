package logic

import (
	"Message/db/redis"
	"Message/log"
	"Message/settings"
	"Message/utils"
	"context"
	"errors"
	"net"
)

const (
	ReadCount = 10
	BlockTime = 10000 //10s
)

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

func ListenMQ(ctx context.Context) {
	for {
		select {
		case <-ctx.Done():
			return
		default:
			msgs, err := redis.ReadFromMQ(ReadCount, BlockTime)
			if err != nil {
				var netErr net.Error
				if errors.As(err, &netErr) && !netErr.Timeout() {
					log.L().Error("Read From MQ", log.Error(err))
				}
				continue
			}
			for _, msg := range msgs {
				m := new(MQMsg)
				m.FromMap(msg.Values)
				log.L().Info("Read From MQ", log.String("msg_id", msg.ID), log.Any("msg", m))

				seq, err := utils.GenID(settings.Conf.MachineID)
				if err != nil {
					log.L().Error("Gen Seq", log.Error(err))
					continue
				}

				con, err := getUserConnection(m.UserID)
				if err != nil {
					log.L().Error("Get User Connection", log.Error(err))
					continue
				}
				con.SendWithACK(seq, m.Cmd, m.MsgType, m.Data, nil)
			}
		}
	}
}
