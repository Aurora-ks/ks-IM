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

// NewGroup 创建群组
func NewGroup(c *gin.Context) {
	var req model.Group
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusOK, Res(ec.BodyParseJson, "NewGroup Parse Json Error"))
		log.L().Warn("NewGroup Body Parse Json")
		return
	}
	gId, err := mysql.NewGroup(req.CreatorID, req.Name, req.Description, req.IsPublic)
	if err != nil {
		log.L().Error("DB New Group", log.Error(err))
		c.JSON(http.StatusOK, Res(ec.DBQuery, "DB New Group"))
		return
	}
	data, err := json.Marshal(model.NewGroupResp{Id: int(gId)})
	if err != nil {
		c.JSON(http.StatusOK, Res(ec.JsonMarshal, "NewGroup Data Marshal Failed"))
		log.L().Error("NewGroup Data Marshal Failed", log.Error(err))
		return
	}
	c.JSON(http.StatusOK, OK(data))
}

// GetGroupInfo 获取群组信息
func GetGroupInfo(c *gin.Context) {
	id := c.Param("group_id")
	gid, err := strconv.Atoi(id)
	if err != nil {
		c.JSON(http.StatusOK, Res(ec.ParmsInvalid, "Group Id Parse Failed"))
		log.L().Error("Group Id Parse Failed", log.Error(err))
		return
	}
	group, err := mysql.GetGroupInfo(gid)
	if err != nil {
		c.JSON(http.StatusOK, Res(ec.DBQuery, "DB Get Group Info"))
		log.L().Error("DB Get Group Info", log.Error(err))
		return
	}
	data, err := json.Marshal(group)
	if err != nil {
		c.JSON(http.StatusOK, Res(ec.JsonMarshal, "Group Info Marshal Failed"))
		log.L().Error("Group Info Marshal Failed", log.Error(err))
		return
	}
	c.JSON(http.StatusOK, OK(data))
}

// GetGroupList 获取用户群组列表
func GetGroupList(c *gin.Context) {
	id := c.Param("user_id")
	uid, err := strconv.Atoi(id)
	if err != nil {
		c.JSON(http.StatusOK, Res(ec.ParmsInvalid, "User Id Parse Failed"))
		log.L().Warn("User Id Parse Failed", log.Error(err))
		return
	}
	groups, err := mysql.GetGroupList(uid)
	if err != nil {
		c.JSON(http.StatusOK, Res(ec.DBQuery, "DB Get Group List"))
		log.L().Error("DB Get Group List", log.Error(err))
		return
	}
	data, err := json.Marshal(groups)
	if err != nil {
		c.JSON(http.StatusOK, Res(ec.JsonMarshal, "Group List Marshal Failed"))
		log.L().Error("Group List Marshal Failed", log.Error(err))
		return
	}
	c.JSON(http.StatusOK, OK(data))
}

// DelGroup 删除群组
func DelGroup(c *gin.Context) {
	var req model.DelGroupReq
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusOK, Res(ec.BodyParseJson, "DelGroup Parse Json Error"))
		log.L().Warn("DelGroup Body Parse Json")
		return
	}
	grp, err := mysql.GetGroupInfo(req.Id)
	if err != nil {
		c.JSON(http.StatusOK, Res(ec.DBQuery, "DB Get Group Info"))
		log.L().Error("DB Get Group Info", log.Error(err))
		return
	}
	if grp.CreatorID != req.UserID {
		c.JSON(http.StatusOK, Res(ec.PermissionDenied, "Permission Denied"))
		log.L().Warn("Delete Group Permission Denied", log.Any("request", req))
		return
	}
	if err = mysql.DelGroup(req.Id); err != nil {
		c.JSON(http.StatusOK, Res(ec.DBDelete, "DB Delete Group"))
		log.L().Error("DB Delete Group", log.Error(err))
		return
	}
	c.JSON(http.StatusOK, OK())
}

// 加入群组
