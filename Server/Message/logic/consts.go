package logic

const (
	CmdACK     = iota // ack
	CmdSC             // single chat
	CmdGC             // group chat
	CmdMsgDelS        // single chat delete
)
const (
	MsgTypeRequest = iota
	MsgTypeResponse
	MsgTypeNotify
	MsgTypePush
	MsgTypeError
	MsgTypeACK
)
