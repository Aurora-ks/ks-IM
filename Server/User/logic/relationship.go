package logic

import (
	"User/db/mysql"
	"User/ec"
	"User/log"
	"User/model"
	"encoding/json"
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

// GetFriendships 获取用户的好友列表
func GetFriendships(c *gin.Context) {
	id := c.Param("uid")
	uid, err := strconv.Atoi(id)
	if err != nil {
		c.JSON(http.StatusOK, Res(ec.ParmsInvalid, "User Id Parse Failed"))
		log.L().Error("User Id Parse Failed", log.Error(err))
		return
	}
	list, err := mysql.GetFriendship(uid, RelStatusPass)
	if err != nil {
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
	id, err := mysql.AddFriend(req.UserID, req.FriendID, req.Remark)
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
	// TODO:推送消息
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
	id := c.Param("uid")
	uid, err := strconv.Atoi(id)
	if err != nil {
		c.JSON(http.StatusOK, Res(ec.ParmsInvalid, "User Id Parse Failed"))
		log.L().Error("User Id Parse Failed", log.Error(err))
	}
	friendList, err := mysql.GetFriendship(uid, RelStatusWait)
	if err != nil {
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
