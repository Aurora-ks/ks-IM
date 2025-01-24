package logic

import (
	"Message/db/mysql"
	"Message/db/redis"
	"Message/log"
	"Message/protocol"
	"Message/settings"
	"Message/utils"
	"strconv"
)

// GroupChatHandler 群聊消息处理
func GroupChatHandler(con *Connection, p *protocol.Packet) {
	switch p.MsgType {
	case MsgTypeRequest:
		requestGC(con, p)
	case MsgTypeACK:
		ackGC(con, p)
	default:
		con.SendError(p.Seq, p.Cmd)
		log.L().Error("Invalid Message Type", log.Any("msg", p))
	}
}

// 请求包处理
func requestGC(con *Connection, p *protocol.Packet) {
	// 数据解包
	m, err := protocol.DecodeMsg(p.Data)
	if err != nil {
		log.L().Error("Decode Msg", log.Error(err), log.Any("msg", p))
		con.SendError(p.Seq, p.Cmd)
		return
	}
	// 判断会话ID是否存在
	if m.ConversationId == 0 {
		// 创建新会话
		cid, err := mysql.CreateNewConversation(m.SenderId, m.ReceiverId, true)
		if err != nil {
			log.L().Error("Create New Conversation", log.Error(err), log.Any("msg", p))
			con.SendError(p.Seq, p.Cmd)
			return
		}
		m.ConversationId = cid
	}
	// 生成消息ID
	msgID, err := utils.GenID(settings.Conf.MachineID)
	if err != nil {
		log.L().Error("Gen Message ID", log.Error(err), log.Any("msg", m))
		con.SendError(p.Seq, p.Cmd)
		return
	}
	m.MsgId = msgID
	// 数据库存储
	if err = mysql.SaveMessage(m, true); err != nil {
		log.L().Error("Save Message", log.Error(err), log.Any("msg", p))
		con.SendError(p.Seq, p.Cmd)
		return
	}
	// 发送ACK附带消息ID
	data, err := protocol.EncodeMsgACKResp(&protocol.MsgACKResponse{MsgId: m.MsgId})
	if err != nil {
		log.L().Error("Encode MsgACKRes_G", log.Error(err), log.Any("msg", p))
		con.SendError(p.Seq, p.Cmd)
		return
	}
	con.Send(p.Seq, CmdSC, MsgTypeACK, data)
	// 发送新消息通知
	GroupChatNotify(m)
}

// GroupChatNotify 群聊消息通知
func GroupChatNotify(m *protocol.Msg) {
	// 通知在线用户拉取最新消息
	// 获取群所有成员
	users, err := mysql.GetGroupMembersID(m.ReceiverId)
	if err != nil {
		log.L().Error("Get Group Members ID", log.Error(err), log.Any("msg", m))
		return
	}
	// 查询在线用户
	for _, uid := range users {
		isOnline, mId, err := redis.GetUserMachineID(strconv.FormatUint(uid, 10))
		if err != nil || !isOnline {
			if err != nil {
				log.L().Error("Redis Check User Online", log.Error(err))
			}
			continue
		}

		data, err := protocol.EncodeGroupMsgNotify(&protocol.GroupMsgNotify{GroupId: m.ReceiverId})
		if err != nil {
			log.L().Error("Encode GroupMsgNotify", log.Error(err))
			continue
		}

		if mId == settings.Conf.ID {
			// 本地转发
			con, err := getUserConnection(uid)
			if err != nil {
				log.L().Error("Get User Connection", log.Error(err))
				continue
			}
			seq, err := utils.GenID(settings.Conf.MachineID)
			if err != nil {
				log.L().Error("Gen Seq", log.Error(err))
				continue
			}
			con.SendWithACK(seq, CmdGC, MsgTypeNotify, data, nil)
		} else {
			// 转发给MQ
			mq := &MQMsg{
				SenderMachID:   settings.Conf.ID,
				ReceivedMachID: mId,
				UserID:         uid,
				Cmd:            CmdGC,
				MsgType:        MsgTypeNotify,
				Data:           data,
			}
			if err := redis.WriteToMQ(mq); err != nil {
				log.L().Error("Write To MQ", log.Error(err), log.Any("mq_msg", mq))
				continue
			}
		}
	}
}

// 收到ACK包处理
func ackGC(con *Connection, p *protocol.Packet) {
	// ack解包
	ack, err := protocol.DecodeMsgACK(p.Data)
	if err != nil {
		log.L().Error("Decode Msg", log.Error(err), log.Any("msg", p))
		con.SendError(p.Seq, p.Cmd)
		return
	}
	defer handleACK(ack.Seq)
	// 获取ack关联的数据包
	pa, err := getPacket(ack.Seq)
	if err != nil {
		log.L().Error("Get Packet", log.Error(err), log.Any("msg", p))
		con.SendError(p.Seq, p.Cmd)
		return
	}
	// 获取关联的消息包
	msg, err := protocol.DecodeMsg(pa.Data)
	if err != nil {
		log.L().Error("Decode Msg", log.Error(err), log.Any("msg", p))
		con.SendError(p.Seq, p.Cmd)
		return
	}
	// 处理ack
	if ack.ConvId == 0 {
		// 创建新会话
		cid, err := mysql.CreateNewConversation(msg.ReceiverId, msg.SenderId, true)
		if err != nil {
			log.L().Error("Create New Conversation", log.Error(err), log.Any("msg", p))
			con.SendError(p.Seq, p.Cmd)
			return
		}
		ack.ConvId = cid
	}
	if err = mysql.UpdateConversation(ack.ConvId, ack.LastMsgId); err != nil {
		log.L().Error("Update Conversation", log.Error(err), log.Any("msg", p))
		con.SendError(p.Seq, p.Cmd)
		return
	}
}
