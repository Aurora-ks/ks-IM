package model

import "time"

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
type RelGrouping struct {
	Id     int    `json:"id"`
	UserID int    `json:"user_id"`
	Name   string `json:"name"`
}
type Group struct {
	Id          int    `json:"id"`
	Name        string `json:"name"`
	CreatorID   int    `json:"creator_id"`
	MemberCount int    `json:"member_count"`
	Description string `json:"description"`
	IsPublic    int    `json:"public"`
}
type GroupMember struct {
	Id      int       `json:"id"`
	GroupId int       `json:"group_id"`
	UserId  int       `json:"user_id"`
	Role    int       `json:"role"`
	JoinAt  time.Time `json:"join_at"`
}
type GroupApply struct {
	Id      int `json:"id"`
	GroupId int `json:"group_id"`
	UserId  int `json:"user_id"`
	Status  int `json:"status"`
}
type GroupSetting struct {
	GroupId    int `json:"group_id"`
	JoinMethod int `json:"join_method"`
}
