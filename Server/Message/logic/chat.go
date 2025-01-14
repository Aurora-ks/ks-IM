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
	case MsgTypeResponse:
		responseSC(con, p)
	case MsgTypeACK:
		ackSC(con, p)
	default:
		log.L().Error("Invalid Message Type", log.Any("msg", p))
	}
}

// SingleChatNotify 单聊消息通知
func SingleChatNotify(m *protocol.Msg) {
	// 查询是否在线
	user, ok := connectionsMap.Load(m.ReceiverId)
	if ok {
		// 在同一服务器中，直接转发
		connection, ok := user.(*Connection)
		if !ok {
			log.L().Error("Get Invalid Type In ConnectionMap", log.Uint64("user_id", m.ReceiverId), log.String("type", reflect.TypeOf(user).String()))
			return
		}
		// 生成序列号
		seq, err := utils.GenID(settings.Conf.MachineID)
		if err != nil {
			log.L().Error("Gen Seq", log.Error(err))
			return
		}
		// 数据编码
		data, err := protocol.EncodeMsg(m)
		if err != nil {
			log.L().Error("Encode Msg", log.Error(err), log.Any("msg", m))
			return
		}
		// 发送
		connection.SendWithACK(uint64(seq), CmdSC, MsgTypeNotify, data, nil)
	} else {
		online, err := redis.IsUserOnline(strconv.FormatUint(m.ReceiverId, 10))
		if err != nil {
			log.L().Error("Redis Check User Online", log.Error(err))
			return
		}
		if online {
			// TODO:转发给对方所在的服务器
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
	m.MsgId = uint64(msgID)
	// 数据库存储
	if err = mysql.SaveMessage(m); err != nil {
		log.L().Error("Save Message", log.Error(err), log.Any("msg", p))
		con.SendError(p.Seq, p.Cmd)
		return
	}
	// 发送ACK附带消息ID
	data, err := protocol.EncodeMsgACKRes_S(&protocol.MsgACKResponse_S{MsgId: m.MsgId})
	if err != nil {
		log.L().Error("Encode MsgACKRes_S", log.Error(err), log.Any("msg", p))
		con.SendError(p.Seq, p.Cmd)
		return
	}
	con.Send(p.Seq, p.Cmd, MsgTypeACK, data)
	// 发送新消息通知
	SingleChatNotify(m)
}

// 收到响应的处理
func responseSC(con *Connection, p *protocol.Packet) {

}

// 收到ACK的处理
func ackSC(con *Connection, p *protocol.Packet) {
	defer handlACK(p.Seq)
	msg, err := protocol.DecodeMsg(p.Data)
	if err != nil {
		log.L().Error("Decode Msg", log.Error(err), log.Any("msg", p))
		con.SendError(p.Seq, p.Cmd)
		return
	}
	if msg.ConversationId == 0 {
		// 创建新会话
		cid, err := mysql.CreateNewConversation(msg.SenderId, msg.ReceiverId, false)
		if err != nil {
			log.L().Error("Create New Conversation", log.Error(err), log.Any("msg", p))
			con.SendError(p.Seq, p.Cmd)
			return
		}
		msg.ConversationId = cid
	}
	if err = mysql.UpdateConversation(msg.ConversationId, msg.MsgId); err != nil {
		log.L().Error("Update Conversation", log.Error(err), log.Any("msg", p))
		con.SendError(p.Seq, p.Cmd)
		return
	}
}
