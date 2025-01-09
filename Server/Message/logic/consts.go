package logic

const (
	CmdACK          = 0x0 // ack
	CmdSC           = 0x1 // single chat
	CmdGC           = 0x2 // group chat
	MsgTypeRequest  = 0x0
	MsgTypeResponse = 0x1
	MsgTypeNotify   = 0x2
	MsgTypePush     = 0x3
	MsgTypeError    = 0x4
	MsgTypeACK      = 0x5
)
