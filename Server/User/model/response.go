package model

import "time"

// user

type RegisterResp struct {
	Id int `json:"id"`
}

// relationship

type AddFriendResp struct {
	Id int `json:"id"`
}
type RespNewFriendRes struct {
	Id     int `json:"id"`
	Option int `json:"option"`
}
type AddRelGroupResp struct {
	Id int `json:"id"`
}
type GetSessionResp struct {
	SessionId int  `json:"session_id"`
	Uid1      int  `json:"uid1"`
	Uid2      int  `json:"uid2"`
	GroupId   int  `json:"group_id"`
	U1LastAck int  `json:"u1_last_ack_msg"`
	U2LastAck int  `json:"u2_last_ack_msg"`
	IsGroup   bool `json:"is_group"`
}
type CreateSessionResp struct {
	SessionId int `json:"session_id"`
}

// group

type NewGroupResp struct {
	Id int `json:"id"`
}

// message

type OfflineMsgResp struct {
	MsgId       int       `json:"msg_id"`
	MsgType     int       `json:"msg_type"`
	Content     string    `json:"text_content"`
	FileName    string    `json:"file_name"`
	FileContent []byte    `json:"file_content"`
	CreateTime  time.Time `json:"create_time"`
}
