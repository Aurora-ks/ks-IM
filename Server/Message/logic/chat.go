package logic

import (
	"Message/db/mysql"
	"Message/log"
	"Message/protocol"
	"reflect"
)

// CreateConversation 创建会话
func CreateConversation(con *Connection, p *protocol.Packet) {
	// 数据库存储
}

// SingleChatRequest 单聊消息处理
func SingleChatRequest(con *Connection, p *protocol.Packet) {
	m, err := protocol.DecodeMsg(p.Data)
	if err != nil {
		log.L().Error("Decode Msg", log.Error(err), log.String("user_id", con.Uid))
		con.SendError(p.Seq, p.Cmd)
		return
	}
	if err = mysql.SaveMessage(m); err != nil {
		log.L().Error("Save Message", log.Error(err), log.String("user_id", con.Uid))
		con.SendError(p.Seq, p.Cmd)
		return
	}
	con.SendACK(p.Seq, p.Cmd)
	SingleChatNotify(con, m)
}

// SingleChatNotify 单聊消息通知
func SingleChatNotify(con *Connection, m *protocol.Msg) {
	// TODO:完善分布式用户查询
	// 查询是否在线
	user, ok := connectionsMap.Load(m.ReceiverId)
	if !ok {
		return
	}
	connection, ok := user.(*Connection)
	if !ok {
		log.L().Error("Get Invalid Type In ConnectionMap", log.Int64("user_id", m.ReceiverId), log.String("type", reflect.TypeOf(user).String()))
		return
	}
	// TODO:生成seq
	connection.Send(0, CmdSC, MsgTypeNotify, m.Content)
}

// UpdateConversation 更新消息会话
func UpdateConversation(m *protocol.Msg) {
	if err := mysql.UpdateConversation(m); err != nil {
		log.L().Error("Update Conversation", log.Error(err), log.Int64("cID", m.ConversationId))
		return
	}
}
