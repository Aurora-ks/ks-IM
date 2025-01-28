package logic

const (
	CmdACK                   = iota // ack
	CmdSC                           // 单聊
	CmdGC                           // 群聊
	CmdMsgDelS                      // 单聊消息删除
	CmdGrpApply                     // 群组加入申请
	CmdGrpApplyResp                 // 群组申请响应
	CmdGrpMemChange                 // 群成员变动
	CmdModifyGroupMemberRole        // 修改群成员角色
	CmdRelApply                     // 好友申请
)
const (
	MsgTypeRequest = iota
	MsgTypeResponse
	MsgTypeNotify
	MsgTypePush
	MsgTypeError
	MsgTypeACK
)
