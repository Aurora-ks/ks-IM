package model

type User struct {
	Id     int    `json:"id"`
	Name   string `json:"name"`
	Gender int    `json:"gender"`
	Icon   string `json:"icon"`
	Email  string `json:"email"`
	Phone  string `json:"phone"`
}
type Image struct {
	Name     string `json:"name"`
	MimeType string `json:"mimeType"`
	Size     int64  `json:"size"`
	Data     string `json:"data"` // Base64 encoded image data
}
type Relationship struct {
	Id       int    `json:"id"`
	UserID   int    `json:"user_id"`
	FriendID int    `json:"friend_id"`
	Status   int    `json:"status"`
	Remark   string `json:"remark"`
	GroupID  int    `json:"group_id"`
}

type RegisterReq struct {
	Name         string `json:"name"`
	Gender       int    `json:"gender"`
	Password     string `json:"password"`
	Email        string `json:"email"`
	Verification string `json:"verification"`
}
type LoginReq struct {
	Id       int    `json:"id"`
	Password string `json:"password"`
}
type GetIconReq struct {
	Id []int `json:"id"`
}
type UpdateIconReq struct {
	Id  int   `json:"id"`
	Img Image `json:"image"`
}
type AddFriendReq struct {
	UserID   int    `json:"user_id"`
	FriendID int    `json:"friend_id"`
	Remark   string `json:"remark"`
}
type AddFriendResp struct {
	Id int `json:"id"`
}
type RespNewFriendRes struct {
	Id     int `json:"id"`
	Option int `json:"option"`
}
