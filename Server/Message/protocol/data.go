package protocol

import (
	"google.golang.org/protobuf/proto"
	"time"
)

func Encode(seq uint64, cmd, msgType uint32, data []byte) ([]byte, error) {
	p := &Packet{
		Seq:        seq,
		Timestamp:  uint64(time.Now().Unix()),
		Cmd:        cmd,
		MsgType:    msgType,
		DataLength: uint32(len(data)),
		Data:       data,
	}
	return proto.Marshal(p)
}

func Decode(data []byte) (p *Packet, err error) {
	p = new(Packet)
	err = proto.Unmarshal(data, p)
	return
}

func EncodeMsg(m *Msg) (data []byte, err error) {
	return proto.Marshal(m)
}
func DecodeMsg(data []byte) (m *Msg, err error) {
	m = new(Msg)
	err = proto.Unmarshal(data, m)
	return
}
func DecodeMsgACK(data []byte) (m *MsgACK_C, err error) {
	m = new(MsgACK_C)
	err = proto.Unmarshal(data, m)
	return
}
func EncodeMsgACK(m *MsgACK_S) ([]byte, error) {
	return proto.Marshal(m)
}
func EncodeGroupMsgNotify(m *GroupMsgNotify) (data []byte, err error) {
	return proto.Marshal(m)
}
