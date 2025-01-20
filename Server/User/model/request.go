package model

// user

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

// relationship

type AddFriendReq struct {
	UserID   int    `json:"user_id"`
	FriendID int    `json:"friend_id"`
	Remark   string `json:"remark"`
}
type AddRelGroupReq struct {
	UserID int    `json:"user_id"`
	Name   string `json:"name"`
}
type ModifyRelGroupReq struct {
	UserID  int    `json:"user_id"`
	Name    string `json:"name"`
	GroupID int    `json:"group_id"`
}
type ModifyFriendGroupReq struct {
	ID      int `json:"id"`
	GroupID int `json:"group_id"`
}
type ModifyFriendAliasReq struct {
	ID    int    `json:"id"`
	Alias string `json:"alias"`
}

// groups

type DelGroupReq struct {
	Id     int `json:"id"`
	UserID int `json:"user_id"`
}
type ApplyJoinGroupReq struct {
	UserID     int `json:"user_id"`
	GroupID    int `json:"group_id"`
	JoinMethod int `json:"join_method"`
}
type GroupJoinApplyDealReq struct {
	ApplyID    int `json:"apply_id"`
	UserID     int `json:"user_id"`
	ReviewerID int `json:"reviewer_id"`
	GroupID    int `json:"group_id"`
	Status     int `json:"status"`
}
type QuitGroupReq struct {
	GroupID int `json:"group_id"`
	UserID  int `json:"user_id"`
}
type ModifyGroupMemberRoleReq struct {
	EditorID int `json:"editor_id"`
	UserID   int `json:"user_id"`
	GroupID  int `json:"group_id"`
	Role     int `json:"role"`
}
