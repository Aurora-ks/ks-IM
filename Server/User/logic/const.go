package logic

const (
	CmdGrpApply              = 4 // 群组加入申请
	CmdGrpApplyResp          = 5 // 群组申请响应
	CmdGrpMemChange          = 6 // 群成员变动
	CmdModifyGroupMemberRole = 7 // 修改群成员角色
	CmdRelApply              = 8 // 好友申请
)

const (
	MsgTypeRequest = iota
	MsgTypeResponse
	MsgTypeNotify
	MsgTypePush
	MsgTypeError
	MsgTypeACK
)
