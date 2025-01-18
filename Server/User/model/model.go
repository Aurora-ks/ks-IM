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
	Alias    string `json:"alias"`
}
