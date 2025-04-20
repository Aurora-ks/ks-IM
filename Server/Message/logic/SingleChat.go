package logic

import (
	"Message/db/mysql"
	"Message/db/redis"
	"Message/log"
	"Message/protocol"
	"Message/settings"
	"Message/utils"
	"reflect"
	"strconv"
)

// SingleChatHandler 单聊消息处理
func SingleChatHandler(con *Connection, p *protocol.Packet) {
	switch p.MsgType {
	case MsgTypeRequest:
		requestSC(con, p)
	case MsgTypeACK:
		ackSC(con, p)
	default:
		con.SendError(p.Seq, p.Cmd)
		log.L().Error("Invalid Message Type", log.Any("msg", p))
	}
}

// SingleChatDelHandler 单聊消息删除
func SingleChatDelHandler(con *Connection, p *protocol.Packet) {
	if p.MsgType != MsgTypeRequest {
		log.L().Error("Invalid Message Type", log.Any("msg", p))
		con.SendError(p.Seq, p.Cmd)
		return
	}
	m, err := protocol.DecodeMsg(p.Data)
	if err != nil {
		log.L().Error("Decode MsgDel_S", log.Error(err), log.Any("msg", p))
		con.SendError(p.Seq, p.Cmd)
		return
	}
	// 删除消息
	err = mysql.DelMsg(m.MsgId)
	if err != nil {
		log.L().Error("Delete Message", log.Error(err), log.Any("msg", p))
		con.SendError(p.Seq, p.Cmd)
		return
	}
	seq, err := utils.GenID(settings.Conf.MachineID)
	if err != nil {
		log.L().Error("Gen Seq", log.Error(err))
		con.SendError(p.Seq, p.Cmd)
		return
	}
	data, err := protocol.EncodeMsgACK(&protocol.MsgACK_S{Seq: p.Seq})
	if err != nil {
		log.L().Error("Encode MsgACK", log.Error(err), log.Any("msg", p))
		con.SendError(p.Seq, p.Cmd)
		return
	}
	// 回复ACK
	con.Send(seq, p.Cmd, MsgTypeACK, data)
	// 查询对方是否在线并通知对方
	online, mId, err := redis.GetUserMachineID(strconv.FormatUint(m.ReceiverId, 10))
	if err != nil || !online {
		if err != nil {
			log.L().Error("Redis Check User Online", log.Error(err))
		}
		return
	}

	if mId == settings.Conf.ID {
		// 在本地服务器中转发
		user, ok := connectionsMap.Load(m.ReceiverId)
		if ok {
			conn, ok := user.(*Connection)
			if !ok {
				log.L().Error("Get Invalid Type In ConnectionMap", log.Uint64("user_id", m.ReceiverId), log.String("type", reflect.TypeOf(user).String()))
				return
			}
			seq, err := utils.GenID(settings.Conf.MachineID)
			if err != nil {
				log.L().Error("Gen Seq", log.Error(err))
				return
			}
			conn.SendWithACK(seq, CmdMsgDelS, MsgTypeNotify, p.Data, nil)
		}
	} else {
		// 转发给相应的MQ
		mqData := &MQMsg{
			SenderMachID:   settings.Conf.ID,
			ReceivedMachID: mId,
			UserID:         m.ReceiverId,
			Cmd:            CmdSC,
			MsgType:        MsgTypeNotify,
			Data:           p.Data,
		}
		if err := redis.WriteToMQ(mqData.ReceivedMachID, mqData.ToMap()); err != nil {
			log.L().Error("Write To MQ", log.Error(err), log.Any("msg", p))
		}
	}
}

// SingleChatNotify 单聊消息通知
func SingleChatNotify(m *protocol.Msg) {
	// 查询是否在线
	isOnline, mId, err := redis.GetUserMachineID(strconv.FormatUint(m.ReceiverId, 10))
	if err != nil || !isOnline {
		if err != nil {
			log.L().Error("Redis Check User Online", log.Error(err))
		}
		return
	}

	data, err := protocol.EncodeMsg(m)
	if err != nil {
		log.L().Error("Encode Msg", log.Error(err), log.Any("msg", m))
		return
	}

	if mId == settings.Conf.ID {
		// 本地转发
		con, err := getUserConnection(m.ReceiverId)
		if err != nil {
			log.L().Error("Get User Connection", log.Error(err))
			return
		}
		seq, err := utils.GenID(settings.Conf.MachineID)
		if err != nil {
			log.L().Error("Gen Seq", log.Error(err))
			return
		}
		con.SendWithACK(seq, CmdSC, MsgTypeNotify, data, nil)
	} else {
		// 转发给MQ
		mq := &MQMsg{
			SenderMachID:   settings.Conf.ID,
			ReceivedMachID: mId,
			UserID:         m.ReceiverId,
			Cmd:            CmdSC,
			MsgType:        MsgTypeNotify,
			Data:           data,
		}
		if err := redis.WriteToMQ(mq.ReceivedMachID, mq.ToMap()); err != nil {
			log.L().Error("Write To MQ", log.Error(err), log.Any("mq_msg", mq))
		}
	}
}

// 收到请求的处理
func requestSC(con *Connection, p *protocol.Packet) {
	// 数据包解码
	m, err := protocol.DecodeMsg(p.Data)
	if err != nil {
		log.L().Error("Decode Msg", log.Error(err), log.Any("msg", p))
		con.SendError(p.Seq, p.Cmd)
		return
	}
	// 判断会话ID是否存在
	if m.ConversationId == 0 {
		// 创建新会话
		cid, err := mysql.CreateNewConversation(m.SenderId, m.ReceiverId, false)
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
	if err = mysql.SaveMessage(m, false); err != nil {
		log.L().Error("Save Message", log.Error(err), log.Any("msg", p))
		con.SendError(p.Seq, p.Cmd)
		return
	}
	// 发送ACK附带消息ID
	seq, err := utils.GenID(settings.Conf.MachineID)
	if err != nil {
		log.L().Error("Gen Seq", log.Error(err))
		con.SendError(p.Seq, p.Cmd)
		return
	}
	data, err := protocol.EncodeMsgACK(&protocol.MsgACK_S{Seq: p.Seq, MsgId: &msgID})
	if err != nil {
		log.L().Error("Encode MsgACKRes_S", log.Error(err), log.Any("msg", p))
		con.SendError(p.Seq, p.Cmd)
		return
	}
	con.Send(seq, CmdACK, MsgTypeACK, data)
	// 发送新消息通知
	SingleChatNotify(m)
}

// 收到ACK的处理
func ackSC(con *Connection, p *protocol.Packet) {
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
	// 处理ACK
	if ack.GetConvId() == 0 {
		// 创建新会话
		cid, err := mysql.CreateNewConversation(msg.ReceiverId, msg.SenderId, false)
		if err != nil {
			log.L().Error("Create New Conversation", log.Error(err), log.Any("msg", p))
			con.SendError(p.Seq, p.Cmd)
			return
		}
		ack.ConvId = &cid
	}
	if err = mysql.UpdateConversation(ack.GetConvId(), ack.GetLastMsgId()); err != nil {
		log.L().Error("Update Conversation", log.Error(err), log.Any("msg", p))
		con.SendError(p.Seq, p.Cmd)
		return
	}
}
