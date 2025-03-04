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

// group

type NewGroupResp struct {
	Id int `json:"id"`
}
