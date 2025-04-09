package model

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
	SessionId int    `json:"session_id"`
	PeerId    int    `json:"peer_id"`
	LastAck   int    `json:"last_ack_msg_id"`
	Name      string `json:"name"`
	IsGroup   bool   `json:"is_group"`
}
type CreateSessionResp struct {
	SessionId int `json:"session_id"`
}

// group

type NewGroupResp struct {
	Id int `json:"id"`
}
