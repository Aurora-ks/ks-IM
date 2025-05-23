package logic

import (
	"User/db/mysql"
	"User/db/redis"
	"User/ec"
	"User/log"
	"User/model"
	"User/settings"
	"database/sql"
	"encoding/json"
	"errors"
	"net/http"
	"strconv"

	"github.com/gin-gonic/gin"
)

const (
	RelStatusWait   = 0
	RelStatusPass   = 1
	RelStatusReject = 2
	RelStatusBlock  = 3
)

// GetFriendGrouping 获取好友分组
func GetFriendGrouping(c *gin.Context) {
	id := c.Query("uid")
	uid, err := strconv.Atoi(id)
	if err != nil {
		c.JSON(http.StatusOK, Res(ec.ParmsInvalid, "User Id Parse Failed"))
		log.L().Error("User Id Parse Failed", log.Error(err))
		return
	}
	list, err := mysql.GetRelGrouping(uid)
	if err != nil && !errors.Is(err, sql.ErrNoRows) {
		log.L().Error("DB Get Friend Grouping", log.Error(err))
		c.JSON(http.StatusOK, Res(ec.DBQuery, "DB Get Friend Grouping"))
		return
	}
	data, err := json.Marshal(list)
	if err != nil {
		log.L().Error("Relationship Data Marshal", log.Error(err))
		c.JSON(http.StatusOK, Res(ec.JsonMarshal, "Relationship Data Marshal Failed"))
		return
	}
	c.JSON(http.StatusOK, OK(data))
}

// GetFriendships 获取用户的好友列表
func GetFriendships(c *gin.Context) {
	id := c.Query("uid")
	uid, err := strconv.Atoi(id)
	if err != nil {
		c.JSON(http.StatusOK, Res(ec.ParmsInvalid, "User Id Parse Failed"))
		log.L().Error("User Id Parse Failed", log.Error(err))
		return
	}
	list, err := mysql.GetFriendship(uid, RelStatusPass)
	if err != nil && !errors.Is(err, sql.ErrNoRows) {
		log.L().Error("DB Get Friend List", log.Error(err))
		c.JSON(http.StatusOK, Res(ec.DBQuery, "DB Get Friend List"))
		return
	}
	data, err := json.Marshal(list)
	if err != nil {
		log.L().Error("Relationship Data Marshal", log.Error(err))
		c.JSON(http.StatusOK, Res(ec.JsonMarshal, "Relationship Data Marshal Failed"))
		return
	}
	c.JSON(http.StatusOK, OK(data))
}

// AddFriend 发送好友申请
func AddFriend(c *gin.Context) {
	var req model.AddFriendReq
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusOK, Res(ec.BodyParseJson, "Add Friend Parse Json Error"))
		log.L().Error("Add Friend Body Parse Json")
		return
	}
	//id, err := mysql.AddFriend(req.UserID, req.FriendID, req.Remark)
	id, err := mysql.AddFriend1(req.UserID, req.FriendID, req.Remark)
	if err != nil {
		c.JSON(http.StatusOK, Res(ec.DBInsert, "Add Friend Failed"))
		log.L().Error("Add Friend Failed", log.Error(err))
		return
	}
	data, err := json.Marshal(model.AddFriendResp{Id: int(id)})
	if err != nil {
		c.JSON(http.StatusOK, Res(ec.JsonMarshal, "Add Friend Data Marshal Failed"))
		log.L().Error("Add Friend Data Marshal Failed", log.Error(err))
		return
	}
	c.JSON(http.StatusOK, OK(data))
	// 推送消息
	online, machID, err := redis.GetUserMachineID(strconv.Itoa(req.FriendID))
	if err != nil || !online {
		if err != nil {
			log.L().Error("Get User Machine ID", log.Error(err))
		}
		return
	}
	if err := redis.WriteToMQ(&redis.MQMsg{
		SenderMachID:   settings.Conf.ID,
		ReceivedMachID: machID,
		UserID:         uint64(req.FriendID),
		Cmd:            CmdRelApply,
		MsgType:        MsgTypeNotify,
		Data:           nil,
	}); err != nil {
		log.L().Error("Write To MQ", log.Error(err))
		return
	}
}

// RespNewFriend 响应好友申请
func RespNewFriend(c *gin.Context) {
	var req model.RespNewFriendRes
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusOK, Res(ec.BodyParseJson, "Add Friend Parse Json Error"))
		log.L().Error("Add Friend Body Parse Json", log.Error(err))
		return
	}
	if err := mysql.RespNewFriend(req.Id, req.Option); err != nil {
		c.JSON(http.StatusOK, Res(ec.DBModify, "Resp New Friend Failed"))
		log.L().Error("Resp New Friend Failed", log.Error(err))
		return
	}
	c.JSON(http.StatusOK, OK())
}

// GetFriendReq 拉取好友申请
func GetFriendReq(c *gin.Context) {
	id := c.Query("uid")
	uid, err := strconv.Atoi(id)
	if err != nil {
		c.JSON(http.StatusOK, Res(ec.ParmsInvalid, "User Id Parse Failed"))
		log.L().Error("User Id Parse Failed", log.Error(err))
	}
	friendList, err := mysql.GetFriendship(uid, RelStatusWait)
	if err != nil && !errors.Is(err, sql.ErrNoRows) {
		c.JSON(http.StatusOK, Res(ec.DBQuery, "DB Get Friend List"))
		log.L().Error("DB Get Friend List", log.Error(err))
		return
	}
	data, err := json.Marshal(friendList)
	if err != nil {
		c.JSON(http.StatusOK, Res(ec.JsonMarshal, "Relationship Data Marshal Failed"))
		log.L().Error("Relationship Data Marshal Failed", log.Error(err))
		return
	}
	c.JSON(http.StatusOK, OK(data))
}

// AddRelGroup 添加关系分组
func AddRelGroup(c *gin.Context) {
	var req model.AddRelGroupReq
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusOK, Res(ec.BodyParseJson, "Add Rel Group Parse Json Error"))
		log.L().Error("Add Rel Group Body Parse Json", log.Error(err))
		return
	}
	id, err := mysql.AddRelGroup(req.UserID, req.Name)
	if err != nil {
		c.JSON(http.StatusOK, Res(ec.DBInsert, "Add Rel Group Failed"))
		log.L().Error("Add Rel Group Failed", log.Error(err))
		return
	}
	data, err := json.Marshal(model.AddRelGroupResp{Id: int(id)})
	if err != nil {
		c.JSON(http.StatusOK, Res(ec.JsonMarshal, "Add Rel Group Data Marshal Failed"))
		log.L().Error("Add Rel Group Data Marshal Failed", log.Error(err))
		return
	}
	c.JSON(http.StatusOK, OK(data))
}

// ModifyRelGroup 修改分组名称
func ModifyRelGroup(c *gin.Context) {
	var req model.ModifyRelGroupReq
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusOK, Res(ec.BodyParseJson, "Modify Rel Group Parse Json Error"))
		log.L().Error("Modify Rel Group Body Parse Json", log.Error(err))
		return
	}
	if err := mysql.ModifyRelGroup(req.GroupID, req.Name); err != nil {
		c.JSON(http.StatusOK, Res(ec.DBModify, "Modify Rel Group Failed"))
		log.L().Error("Modify Rel Group Failed", log.Error(err))
		return
	}
	c.JSON(http.StatusOK, OK())
}

// ModifyFriendGroup 修改好友分组
func ModifyFriendGroup(c *gin.Context) {
	var req model.ModifyFriendGroupReq
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusOK, Res(ec.BodyParseJson, "Modify Friend Group Parse Json Error"))
		log.L().Error("Modify Friend Group Body Parse Json", log.Error(err))
		return
	}
	if err := mysql.ModifyFriendGroup(req.ID, req.GroupID); err != nil {
		c.JSON(http.StatusOK, Res(ec.DBModify, "Modify Friend Group Failed"))
		log.L().Error("Modify Friend Group Failed", log.Error(err))
		return
	}
	c.JSON(http.StatusOK, OK())
}

// ModifyFriendAlias 修改好友备注
func ModifyFriendAlias(c *gin.Context) {
	var req model.ModifyFriendAliasReq
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusOK, Res(ec.BodyParseJson, "Modify Friend Alias Parse Json Error"))
		log.L().Error("Modify Friend Alias Body Parse Json", log.Error(err))
		return
	}
	if err := mysql.ModifyFriendAlias(req.ID, req.Alias); err != nil {
		c.JSON(http.StatusOK, Res(ec.DBModify, "Modify Friend Alias Failed"))
		log.L().Error("Modify Friend Alias Failed", log.Error(err))
		return
	}
	c.JSON(http.StatusOK, OK())
}

// GetSession 获取会话
func GetSession(c *gin.Context) {
	id := c.Query("user_id")
	uid, err := strconv.Atoi(id)
	if err != nil {
		c.JSON(http.StatusOK, Res(ec.ParmsInvalid, "User Id Parse Failed"))
		log.L().Error("User Id Parse Failed", log.Error(err))
		return
	}
	isGroup := c.Query("is_group")
	var bIsGroup bool
	if isGroup == "true" {
		bIsGroup = true
	} else {
		bIsGroup = false
	}

	sessionList, err := mysql.GetSession(uid, bIsGroup)
	if err != nil {
		c.JSON(http.StatusOK, Res(ec.DBQuery, "Get Session Failed"))
		log.L().Error("Get Session Failed", log.Error(err))
		return
	}
	data, err := json.Marshal(sessionList)
	if err != nil {
		c.JSON(http.StatusOK, Res(ec.JsonMarshal, "Session List Marshal Failed"))
		log.L().Error("Session List Marshal Failed", log.Error(err))
		return
	}
	c.JSON(http.StatusOK, OK(data))
}

// CreateSession 创建会话
func CreateSession(c *gin.Context) {
	var req model.CreateSessionReq
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusOK, Res(ec.BodyParseJson, "Create Session Parse Json Error"))
		log.L().Error("Create Session Body Parse Json", log.Error(err))
		return
	}
	resp, err := mysql.NewSession(req.UserID, req.PeerID, req.IsGroup)
	if err != nil {
		c.JSON(http.StatusOK, Res(ec.DBInsert, "Create Session Failed"))
		log.L().Error("Create Session Failed", log.Error(err))
		return
	}
	data, err := json.Marshal(resp)
	if err != nil {
		c.JSON(http.StatusOK, Res(ec.JsonMarshal, "Session Data Marshal Failed"))
		log.L().Error("Session Data Marshal Failed", log.Error(err))
		return
	}
	c.JSON(http.StatusOK, OK(data))
}
