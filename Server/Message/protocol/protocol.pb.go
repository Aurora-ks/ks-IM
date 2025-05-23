// Code generated by protoc-gen-go. DO NOT EDIT.
// versions:
// 	protoc-gen-go v1.36.0
// 	protoc        v5.29.1
// source: protocol/protocol.proto

package protocol

import (
	protoreflect "google.golang.org/protobuf/reflect/protoreflect"
	protoimpl "google.golang.org/protobuf/runtime/protoimpl"
	reflect "reflect"
	sync "sync"
)

const (
	// Verify that this generated code is sufficiently up-to-date.
	_ = protoimpl.EnforceVersion(20 - protoimpl.MinVersion)
	// Verify that runtime/protoimpl is sufficiently up-to-date.
	_ = protoimpl.EnforceVersion(protoimpl.MaxVersion - 20)
)

// 28 Bytes Header
type Packet struct {
	state         protoimpl.MessageState `protogen:"open.v1"`
	Seq           uint64                 `protobuf:"fixed64,1,opt,name=seq,proto3" json:"seq,omitempty"`
	Timestamp     uint64                 `protobuf:"fixed64,2,opt,name=timestamp,proto3" json:"timestamp,omitempty"`
	Cmd           uint32                 `protobuf:"fixed32,3,opt,name=cmd,proto3" json:"cmd,omitempty"`
	MsgType       uint32                 `protobuf:"fixed32,4,opt,name=msg_type,json=msgType,proto3" json:"msg_type,omitempty"`
	DataLength    uint32                 `protobuf:"fixed32,5,opt,name=data_length,json=dataLength,proto3" json:"data_length,omitempty"`
	Data          []byte                 `protobuf:"bytes,6,opt,name=data,proto3" json:"data,omitempty"`
	unknownFields protoimpl.UnknownFields
	sizeCache     protoimpl.SizeCache
}

func (x *Packet) Reset() {
	*x = Packet{}
	mi := &file_protocol_protocol_proto_msgTypes[0]
	ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
	ms.StoreMessageInfo(mi)
}

func (x *Packet) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*Packet) ProtoMessage() {}

func (x *Packet) ProtoReflect() protoreflect.Message {
	mi := &file_protocol_protocol_proto_msgTypes[0]
	if x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use Packet.ProtoReflect.Descriptor instead.
func (*Packet) Descriptor() ([]byte, []int) {
	return file_protocol_protocol_proto_rawDescGZIP(), []int{0}
}

func (x *Packet) GetSeq() uint64 {
	if x != nil {
		return x.Seq
	}
	return 0
}

func (x *Packet) GetTimestamp() uint64 {
	if x != nil {
		return x.Timestamp
	}
	return 0
}

func (x *Packet) GetCmd() uint32 {
	if x != nil {
		return x.Cmd
	}
	return 0
}

func (x *Packet) GetMsgType() uint32 {
	if x != nil {
		return x.MsgType
	}
	return 0
}

func (x *Packet) GetDataLength() uint32 {
	if x != nil {
		return x.DataLength
	}
	return 0
}

func (x *Packet) GetData() []byte {
	if x != nil {
		return x.Data
	}
	return nil
}

// 消息结构
type Msg struct {
	state          protoimpl.MessageState `protogen:"open.v1"`
	MsgId          uint64                 `protobuf:"varint,1,opt,name=msg_id,json=msgId,proto3" json:"msg_id,omitempty"`
	ConversationId uint64                 `protobuf:"varint,2,opt,name=conversation_id,json=conversationId,proto3" json:"conversation_id,omitempty"`
	SenderId       uint64                 `protobuf:"varint,3,opt,name=sender_id,json=senderId,proto3" json:"sender_id,omitempty"`
	ReceiverId     uint64                 `protobuf:"varint,4,opt,name=receiver_id,json=receiverId,proto3" json:"receiver_id,omitempty"`
	ContentType    uint32                 `protobuf:"varint,5,opt,name=content_type,json=contentType,proto3" json:"content_type,omitempty"` //0:text, 1:image, 2:file
	IsGroup        bool                   `protobuf:"varint,6,opt,name=is_group,json=isGroup,proto3" json:"is_group,omitempty"`
	Content        []byte                 `protobuf:"bytes,7,opt,name=content,proto3" json:"content,omitempty"`
	FileName       *string                `protobuf:"bytes,8,opt,name=file_name,json=fileName,proto3,oneof" json:"file_name,omitempty"`
	unknownFields  protoimpl.UnknownFields
	sizeCache      protoimpl.SizeCache
}

func (x *Msg) Reset() {
	*x = Msg{}
	mi := &file_protocol_protocol_proto_msgTypes[1]
	ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
	ms.StoreMessageInfo(mi)
}

func (x *Msg) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*Msg) ProtoMessage() {}

func (x *Msg) ProtoReflect() protoreflect.Message {
	mi := &file_protocol_protocol_proto_msgTypes[1]
	if x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use Msg.ProtoReflect.Descriptor instead.
func (*Msg) Descriptor() ([]byte, []int) {
	return file_protocol_protocol_proto_rawDescGZIP(), []int{1}
}

func (x *Msg) GetMsgId() uint64 {
	if x != nil {
		return x.MsgId
	}
	return 0
}

func (x *Msg) GetConversationId() uint64 {
	if x != nil {
		return x.ConversationId
	}
	return 0
}

func (x *Msg) GetSenderId() uint64 {
	if x != nil {
		return x.SenderId
	}
	return 0
}

func (x *Msg) GetReceiverId() uint64 {
	if x != nil {
		return x.ReceiverId
	}
	return 0
}

func (x *Msg) GetContentType() uint32 {
	if x != nil {
		return x.ContentType
	}
	return 0
}

func (x *Msg) GetIsGroup() bool {
	if x != nil {
		return x.IsGroup
	}
	return false
}

func (x *Msg) GetContent() []byte {
	if x != nil {
		return x.Content
	}
	return nil
}

func (x *Msg) GetFileName() string {
	if x != nil && x.FileName != nil {
		return *x.FileName
	}
	return ""
}

// 客户端发送的ACK包
type MsgACK_C struct {
	state         protoimpl.MessageState `protogen:"open.v1"`
	Seq           uint64                 `protobuf:"varint,1,opt,name=seq,proto3" json:"seq,omitempty"`
	ConvId        *uint64                `protobuf:"varint,2,opt,name=conv_id,json=convId,proto3,oneof" json:"conv_id,omitempty"`
	LastMsgId     *uint64                `protobuf:"varint,3,opt,name=last_msg_id,json=lastMsgId,proto3,oneof" json:"last_msg_id,omitempty"`
	unknownFields protoimpl.UnknownFields
	sizeCache     protoimpl.SizeCache
}

func (x *MsgACK_C) Reset() {
	*x = MsgACK_C{}
	mi := &file_protocol_protocol_proto_msgTypes[2]
	ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
	ms.StoreMessageInfo(mi)
}

func (x *MsgACK_C) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*MsgACK_C) ProtoMessage() {}

func (x *MsgACK_C) ProtoReflect() protoreflect.Message {
	mi := &file_protocol_protocol_proto_msgTypes[2]
	if x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use MsgACK_C.ProtoReflect.Descriptor instead.
func (*MsgACK_C) Descriptor() ([]byte, []int) {
	return file_protocol_protocol_proto_rawDescGZIP(), []int{2}
}

func (x *MsgACK_C) GetSeq() uint64 {
	if x != nil {
		return x.Seq
	}
	return 0
}

func (x *MsgACK_C) GetConvId() uint64 {
	if x != nil && x.ConvId != nil {
		return *x.ConvId
	}
	return 0
}

func (x *MsgACK_C) GetLastMsgId() uint64 {
	if x != nil && x.LastMsgId != nil {
		return *x.LastMsgId
	}
	return 0
}

// 服务端发送的ACK包
type MsgACK_S struct {
	state         protoimpl.MessageState `protogen:"open.v1"`
	Seq           uint64                 `protobuf:"varint,1,opt,name=seq,proto3" json:"seq,omitempty"`
	MsgId         *uint64                `protobuf:"varint,2,opt,name=msg_id,json=msgId,proto3,oneof" json:"msg_id,omitempty"`
	unknownFields protoimpl.UnknownFields
	sizeCache     protoimpl.SizeCache
}

func (x *MsgACK_S) Reset() {
	*x = MsgACK_S{}
	mi := &file_protocol_protocol_proto_msgTypes[3]
	ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
	ms.StoreMessageInfo(mi)
}

func (x *MsgACK_S) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*MsgACK_S) ProtoMessage() {}

func (x *MsgACK_S) ProtoReflect() protoreflect.Message {
	mi := &file_protocol_protocol_proto_msgTypes[3]
	if x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use MsgACK_S.ProtoReflect.Descriptor instead.
func (*MsgACK_S) Descriptor() ([]byte, []int) {
	return file_protocol_protocol_proto_rawDescGZIP(), []int{3}
}

func (x *MsgACK_S) GetSeq() uint64 {
	if x != nil {
		return x.Seq
	}
	return 0
}

func (x *MsgACK_S) GetMsgId() uint64 {
	if x != nil && x.MsgId != nil {
		return *x.MsgId
	}
	return 0
}

// 服务端群聊通知
type GroupMsgNotify struct {
	state         protoimpl.MessageState `protogen:"open.v1"`
	GroupId       uint64                 `protobuf:"varint,1,opt,name=group_id,json=groupId,proto3" json:"group_id,omitempty"`
	unknownFields protoimpl.UnknownFields
	sizeCache     protoimpl.SizeCache
}

func (x *GroupMsgNotify) Reset() {
	*x = GroupMsgNotify{}
	mi := &file_protocol_protocol_proto_msgTypes[4]
	ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
	ms.StoreMessageInfo(mi)
}

func (x *GroupMsgNotify) String() string {
	return protoimpl.X.MessageStringOf(x)
}

func (*GroupMsgNotify) ProtoMessage() {}

func (x *GroupMsgNotify) ProtoReflect() protoreflect.Message {
	mi := &file_protocol_protocol_proto_msgTypes[4]
	if x != nil {
		ms := protoimpl.X.MessageStateOf(protoimpl.Pointer(x))
		if ms.LoadMessageInfo() == nil {
			ms.StoreMessageInfo(mi)
		}
		return ms
	}
	return mi.MessageOf(x)
}

// Deprecated: Use GroupMsgNotify.ProtoReflect.Descriptor instead.
func (*GroupMsgNotify) Descriptor() ([]byte, []int) {
	return file_protocol_protocol_proto_rawDescGZIP(), []int{4}
}

func (x *GroupMsgNotify) GetGroupId() uint64 {
	if x != nil {
		return x.GroupId
	}
	return 0
}

var File_protocol_protocol_proto protoreflect.FileDescriptor

var file_protocol_protocol_proto_rawDesc = []byte{
	0x0a, 0x17, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x63, 0x6f, 0x6c, 0x2f, 0x70, 0x72, 0x6f, 0x74, 0x6f,
	0x63, 0x6f, 0x6c, 0x2e, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x12, 0x08, 0x70, 0x72, 0x6f, 0x74, 0x6f,
	0x63, 0x6f, 0x6c, 0x22, 0x9a, 0x01, 0x0a, 0x06, 0x50, 0x61, 0x63, 0x6b, 0x65, 0x74, 0x12, 0x10,
	0x0a, 0x03, 0x73, 0x65, 0x71, 0x18, 0x01, 0x20, 0x01, 0x28, 0x06, 0x52, 0x03, 0x73, 0x65, 0x71,
	0x12, 0x1c, 0x0a, 0x09, 0x74, 0x69, 0x6d, 0x65, 0x73, 0x74, 0x61, 0x6d, 0x70, 0x18, 0x02, 0x20,
	0x01, 0x28, 0x06, 0x52, 0x09, 0x74, 0x69, 0x6d, 0x65, 0x73, 0x74, 0x61, 0x6d, 0x70, 0x12, 0x10,
	0x0a, 0x03, 0x63, 0x6d, 0x64, 0x18, 0x03, 0x20, 0x01, 0x28, 0x07, 0x52, 0x03, 0x63, 0x6d, 0x64,
	0x12, 0x19, 0x0a, 0x08, 0x6d, 0x73, 0x67, 0x5f, 0x74, 0x79, 0x70, 0x65, 0x18, 0x04, 0x20, 0x01,
	0x28, 0x07, 0x52, 0x07, 0x6d, 0x73, 0x67, 0x54, 0x79, 0x70, 0x65, 0x12, 0x1f, 0x0a, 0x0b, 0x64,
	0x61, 0x74, 0x61, 0x5f, 0x6c, 0x65, 0x6e, 0x67, 0x74, 0x68, 0x18, 0x05, 0x20, 0x01, 0x28, 0x07,
	0x52, 0x0a, 0x64, 0x61, 0x74, 0x61, 0x4c, 0x65, 0x6e, 0x67, 0x74, 0x68, 0x12, 0x12, 0x0a, 0x04,
	0x64, 0x61, 0x74, 0x61, 0x18, 0x06, 0x20, 0x01, 0x28, 0x0c, 0x52, 0x04, 0x64, 0x61, 0x74, 0x61,
	0x22, 0x8b, 0x02, 0x0a, 0x03, 0x4d, 0x73, 0x67, 0x12, 0x15, 0x0a, 0x06, 0x6d, 0x73, 0x67, 0x5f,
	0x69, 0x64, 0x18, 0x01, 0x20, 0x01, 0x28, 0x04, 0x52, 0x05, 0x6d, 0x73, 0x67, 0x49, 0x64, 0x12,
	0x27, 0x0a, 0x0f, 0x63, 0x6f, 0x6e, 0x76, 0x65, 0x72, 0x73, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x5f,
	0x69, 0x64, 0x18, 0x02, 0x20, 0x01, 0x28, 0x04, 0x52, 0x0e, 0x63, 0x6f, 0x6e, 0x76, 0x65, 0x72,
	0x73, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x49, 0x64, 0x12, 0x1b, 0x0a, 0x09, 0x73, 0x65, 0x6e, 0x64,
	0x65, 0x72, 0x5f, 0x69, 0x64, 0x18, 0x03, 0x20, 0x01, 0x28, 0x04, 0x52, 0x08, 0x73, 0x65, 0x6e,
	0x64, 0x65, 0x72, 0x49, 0x64, 0x12, 0x1f, 0x0a, 0x0b, 0x72, 0x65, 0x63, 0x65, 0x69, 0x76, 0x65,
	0x72, 0x5f, 0x69, 0x64, 0x18, 0x04, 0x20, 0x01, 0x28, 0x04, 0x52, 0x0a, 0x72, 0x65, 0x63, 0x65,
	0x69, 0x76, 0x65, 0x72, 0x49, 0x64, 0x12, 0x21, 0x0a, 0x0c, 0x63, 0x6f, 0x6e, 0x74, 0x65, 0x6e,
	0x74, 0x5f, 0x74, 0x79, 0x70, 0x65, 0x18, 0x05, 0x20, 0x01, 0x28, 0x0d, 0x52, 0x0b, 0x63, 0x6f,
	0x6e, 0x74, 0x65, 0x6e, 0x74, 0x54, 0x79, 0x70, 0x65, 0x12, 0x19, 0x0a, 0x08, 0x69, 0x73, 0x5f,
	0x67, 0x72, 0x6f, 0x75, 0x70, 0x18, 0x06, 0x20, 0x01, 0x28, 0x08, 0x52, 0x07, 0x69, 0x73, 0x47,
	0x72, 0x6f, 0x75, 0x70, 0x12, 0x18, 0x0a, 0x07, 0x63, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74, 0x18,
	0x07, 0x20, 0x01, 0x28, 0x0c, 0x52, 0x07, 0x63, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74, 0x12, 0x20,
	0x0a, 0x09, 0x66, 0x69, 0x6c, 0x65, 0x5f, 0x6e, 0x61, 0x6d, 0x65, 0x18, 0x08, 0x20, 0x01, 0x28,
	0x09, 0x48, 0x00, 0x52, 0x08, 0x66, 0x69, 0x6c, 0x65, 0x4e, 0x61, 0x6d, 0x65, 0x88, 0x01, 0x01,
	0x42, 0x0c, 0x0a, 0x0a, 0x5f, 0x66, 0x69, 0x6c, 0x65, 0x5f, 0x6e, 0x61, 0x6d, 0x65, 0x22, 0x7b,
	0x0a, 0x08, 0x4d, 0x73, 0x67, 0x41, 0x43, 0x4b, 0x5f, 0x43, 0x12, 0x10, 0x0a, 0x03, 0x73, 0x65,
	0x71, 0x18, 0x01, 0x20, 0x01, 0x28, 0x04, 0x52, 0x03, 0x73, 0x65, 0x71, 0x12, 0x1c, 0x0a, 0x07,
	0x63, 0x6f, 0x6e, 0x76, 0x5f, 0x69, 0x64, 0x18, 0x02, 0x20, 0x01, 0x28, 0x04, 0x48, 0x00, 0x52,
	0x06, 0x63, 0x6f, 0x6e, 0x76, 0x49, 0x64, 0x88, 0x01, 0x01, 0x12, 0x23, 0x0a, 0x0b, 0x6c, 0x61,
	0x73, 0x74, 0x5f, 0x6d, 0x73, 0x67, 0x5f, 0x69, 0x64, 0x18, 0x03, 0x20, 0x01, 0x28, 0x04, 0x48,
	0x01, 0x52, 0x09, 0x6c, 0x61, 0x73, 0x74, 0x4d, 0x73, 0x67, 0x49, 0x64, 0x88, 0x01, 0x01, 0x42,
	0x0a, 0x0a, 0x08, 0x5f, 0x63, 0x6f, 0x6e, 0x76, 0x5f, 0x69, 0x64, 0x42, 0x0e, 0x0a, 0x0c, 0x5f,
	0x6c, 0x61, 0x73, 0x74, 0x5f, 0x6d, 0x73, 0x67, 0x5f, 0x69, 0x64, 0x22, 0x43, 0x0a, 0x08, 0x4d,
	0x73, 0x67, 0x41, 0x43, 0x4b, 0x5f, 0x53, 0x12, 0x10, 0x0a, 0x03, 0x73, 0x65, 0x71, 0x18, 0x01,
	0x20, 0x01, 0x28, 0x04, 0x52, 0x03, 0x73, 0x65, 0x71, 0x12, 0x1a, 0x0a, 0x06, 0x6d, 0x73, 0x67,
	0x5f, 0x69, 0x64, 0x18, 0x02, 0x20, 0x01, 0x28, 0x04, 0x48, 0x00, 0x52, 0x05, 0x6d, 0x73, 0x67,
	0x49, 0x64, 0x88, 0x01, 0x01, 0x42, 0x09, 0x0a, 0x07, 0x5f, 0x6d, 0x73, 0x67, 0x5f, 0x69, 0x64,
	0x22, 0x2b, 0x0a, 0x0e, 0x47, 0x72, 0x6f, 0x75, 0x70, 0x4d, 0x73, 0x67, 0x4e, 0x6f, 0x74, 0x69,
	0x66, 0x79, 0x12, 0x19, 0x0a, 0x08, 0x67, 0x72, 0x6f, 0x75, 0x70, 0x5f, 0x69, 0x64, 0x18, 0x01,
	0x20, 0x01, 0x28, 0x04, 0x52, 0x07, 0x67, 0x72, 0x6f, 0x75, 0x70, 0x49, 0x64, 0x42, 0x0d, 0x5a,
	0x0b, 0x2e, 0x2f, 0x3b, 0x70, 0x72, 0x6f, 0x74, 0x6f, 0x63, 0x6f, 0x6c, 0x62, 0x06, 0x70, 0x72,
	0x6f, 0x74, 0x6f, 0x33,
}

var (
	file_protocol_protocol_proto_rawDescOnce sync.Once
	file_protocol_protocol_proto_rawDescData = file_protocol_protocol_proto_rawDesc
)

func file_protocol_protocol_proto_rawDescGZIP() []byte {
	file_protocol_protocol_proto_rawDescOnce.Do(func() {
		file_protocol_protocol_proto_rawDescData = protoimpl.X.CompressGZIP(file_protocol_protocol_proto_rawDescData)
	})
	return file_protocol_protocol_proto_rawDescData
}

var file_protocol_protocol_proto_msgTypes = make([]protoimpl.MessageInfo, 5)
var file_protocol_protocol_proto_goTypes = []any{
	(*Packet)(nil),         // 0: protocol.Packet
	(*Msg)(nil),            // 1: protocol.Msg
	(*MsgACK_C)(nil),       // 2: protocol.MsgACK_C
	(*MsgACK_S)(nil),       // 3: protocol.MsgACK_S
	(*GroupMsgNotify)(nil), // 4: protocol.GroupMsgNotify
}
var file_protocol_protocol_proto_depIdxs = []int32{
	0, // [0:0] is the sub-list for method output_type
	0, // [0:0] is the sub-list for method input_type
	0, // [0:0] is the sub-list for extension type_name
	0, // [0:0] is the sub-list for extension extendee
	0, // [0:0] is the sub-list for field type_name
}

func init() { file_protocol_protocol_proto_init() }
func file_protocol_protocol_proto_init() {
	if File_protocol_protocol_proto != nil {
		return
	}
	file_protocol_protocol_proto_msgTypes[1].OneofWrappers = []any{}
	file_protocol_protocol_proto_msgTypes[2].OneofWrappers = []any{}
	file_protocol_protocol_proto_msgTypes[3].OneofWrappers = []any{}
	type x struct{}
	out := protoimpl.TypeBuilder{
		File: protoimpl.DescBuilder{
			GoPackagePath: reflect.TypeOf(x{}).PkgPath(),
			RawDescriptor: file_protocol_protocol_proto_rawDesc,
			NumEnums:      0,
			NumMessages:   5,
			NumExtensions: 0,
			NumServices:   0,
		},
		GoTypes:           file_protocol_protocol_proto_goTypes,
		DependencyIndexes: file_protocol_protocol_proto_depIdxs,
		MessageInfos:      file_protocol_protocol_proto_msgTypes,
	}.Build()
	File_protocol_protocol_proto = out.File
	file_protocol_protocol_proto_rawDesc = nil
	file_protocol_protocol_proto_goTypes = nil
	file_protocol_protocol_proto_depIdxs = nil
}
