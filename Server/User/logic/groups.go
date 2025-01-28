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

const GrpJoinMethInvite = 1

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
	id := c.Query("group_id")
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

// GetGroupList 获取用户加入的群组列表
func GetGroupList(c *gin.Context) {
	id := c.Query("uid")
	uid, err := strconv.Atoi(id)
	if err != nil {
		c.JSON(http.StatusOK, Res(ec.ParmsInvalid, "User Id Parse Failed"))
		log.L().Warn("User Id Parse Failed", log.Error(err))
		return
	}
	groups, err := mysql.GetGroupList(uid, mysql.GrpRoleAll)
	if err != nil && !errors.Is(err, sql.ErrNoRows) {
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

// ApplyJoinGroup 申请加入群组
func ApplyJoinGroup(c *gin.Context) {
	var req model.ApplyJoinGroupReq
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusOK, Res(ec.BodyParseJson, "ApplyJoinGroup Parse Json Error"))
		log.L().Warn("ApplyJoinGroup Body Parse Json")
		return
	}
	// 查询群加入方式
	grpSetting, err := mysql.GetGroupSetting(req.GroupID)
	if err != nil {
		c.JSON(http.StatusOK, Res(ec.DBQuery, "DB Get Group Setting"))
		log.L().Error("DB Get Group Setting", log.Error(err))
		return
	} else if grpSetting.JoinMethod == mysql.GrpJoinMethInvite && req.JoinMethod != GrpJoinMethInvite {
		c.JSON(http.StatusOK, Res(ec.PermissionDenied, "Permission Denied"))
		log.L().Warn("Apply Join Group Permission Denied", log.Any("request", req))
	}
	// 可直接加入
	if grpSetting.JoinMethod == mysql.GrpJoinMethAll {
		err = addGroupNewMember([]int{req.UserID}, req.GroupID)
		if err != nil {
			c.JSON(http.StatusOK, Res(ec.DBInsert, "DB Apply Join Group"))
			log.L().Error("DB Apply Join Group", log.Error(err))
			return
		}
	} else {
		// 需要审核
		if err = mysql.ApplyJoinGroup(req.GroupID, req.UserID); err != nil {
			c.JSON(http.StatusOK, Res(ec.DBInsert, "DB Apply Join Group"))
			log.L().Error("DB Apply Join Group", log.Error(err))
			return
		}
		// 获取所有在线的群管理
		// 推送消息
		online, machID, err := redis.GetUserMachineID(strconv.Itoa(req.UserID))
		if err != nil || !online {
			if err != nil {
				log.L().Error("Get User Machine ID", log.Error(err))
			}
			return
		}
		// 推送消息队列
		if err := redis.WriteToMQ(&MQMsg{
			SenderMachID:   settings.Conf.ID,
			ReceivedMachID: machID,
			UserID:         uint64(req.UserID),
			Cmd:            CmdGrpApply,
			MsgType:        MsgTypeNotify,
			Data:           nil,
		}); err != nil {
			log.L().Error("Write To MQ", log.Error(err))
			return
		}
	}
}

// GetGroupApplyList 获取群组申请列表
func GetGroupApplyList(c *gin.Context) {
	id := c.Query("uid")
	uid, err := strconv.Atoi(id)
	if err != nil {
		c.JSON(http.StatusOK, Res(ec.ParmsInvalid, "User Id Parse Failed"))
		log.L().Warn("User Id Parse Failed", log.Error(err))
		return
	}
	var grpList []int
	// 获取自身的申请列表
	applyList, err := mysql.GetGroupApplyListWithUser(uid)
	if err != nil && !errors.Is(err, sql.ErrNoRows) {
		c.JSON(http.StatusOK, Res(ec.DBQuery, "DB Get Group Apply List"))
		log.L().Error("DB Get Group Apply List", log.Error(err))
		return
	}
	for _, v := range applyList {
		grpList = append(grpList, v.GroupId)
	}
	// 获取用户管理的所有群id
	l, err := mysql.GetGroupList(uid, mysql.GrpRoleOwner)
	if err != nil && !errors.Is(err, sql.ErrNoRows) {
		c.JSON(http.StatusOK, Res(ec.DBQuery, "DB Get Group Apply List"))
		log.L().Error("DB Get Group Apply List", log.Error(err))
		return
	}
	for _, v := range l {
		grpList = append(grpList, v.GroupId)
	}
	l, err = mysql.GetGroupList(uid, mysql.GrpRoleAdmin)
	if err != nil && !errors.Is(err, sql.ErrNoRows) {
		c.JSON(http.StatusOK, Res(ec.DBQuery, "DB Get Group Apply List"))
		log.L().Error("DB Get Group Apply List", log.Error(err))
		return
	}
	for _, v := range l {
		grpList = append(grpList, v.GroupId)
	}
	// 查询申请列表
	applyList, err = mysql.GetGroupApplyListWithGroup(grpList)
	if err != nil && !errors.Is(err, sql.ErrNoRows) {
		c.JSON(http.StatusOK, Res(ec.DBQuery, "DB Get Group Apply List"))
		log.L().Error("DB Get Group Apply List", log.Error(err))
		return
	}

	data, err := json.Marshal(applyList)
	if err != nil {
		c.JSON(http.StatusOK, Res(ec.JsonMarshal, "Group Apply List Marshal Failed"))
		log.L().Error("Group Apply List Marshal Failed", log.Error(err))
		return
	}
	c.JSON(http.StatusOK, OK(data))
}

// GroupJoinApplyDeal 群组申请处理
func GroupJoinApplyDeal(c *gin.Context) {
	var req model.GroupJoinApplyDealReq
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusOK, Res(ec.BodyParseJson, "GroupJoinApplyHandle Parse Json Error"))
		log.L().Warn("GroupJoinApplyHandle Body Parse Json")
		return
	}
	// 权限校验
	info, err := mysql.GetGroupMemberInfo(req.UserID, req.GroupID)
	if err != nil || info.Role != mysql.GrpRoleOwner && info.Role != mysql.GrpRoleAdmin {
		c.JSON(http.StatusOK, Res(ec.PermissionDenied, "Permission Denied"))
		log.L().Warn("GroupJoinApplyHandle Permission Denied", log.Any("request", req))
		return
	}
	// 处理申请
	if err = mysql.GroupJoinApplyDeal(req.ApplyID, req.UserID, req.GroupID, req.Status); err != nil {
		c.JSON(http.StatusOK, Res(ec.DBModify, "DB Group Join Apply Deal"))
		log.L().Error("DB Group Join Apply Deal", log.Error(err))
		return
	}
	c.JSON(http.StatusOK, OK())
	// 给申请人推送消息
	online, machID, err := redis.GetUserMachineID(strconv.Itoa(req.UserID))
	if err != nil || !online {
		if err != nil {
			log.L().Error("Get User Machine ID", log.Error(err))
		}
		return
	}
	if err := redis.WriteToMQ(&MQMsg{
		SenderMachID:   settings.Conf.ID,
		ReceivedMachID: machID,
		UserID:         uint64(req.UserID),
		Cmd:            CmdGrpApplyResp,
		MsgType:        MsgTypeNotify,
		Data:           nil,
	}); err != nil {
		log.L().Error("Write To MQ", log.Error(err))
		return
	}
	// 给群在线成员推送消息
	members, err := mysql.GetGroupMemberList(req.GroupID, mysql.GrpRoleAll)
	if err != nil {
		log.L().Error("Get Group Member List", log.Error(err))
		return
	}
	for _, mem := range members {
		online, machID, err := redis.GetUserMachineID(strconv.Itoa(mem.UserId))
		if err != nil || !online {
			if err != nil {
				log.L().Error("Get User Machine ID", log.Error(err))
			}
			continue
		}
		if err := redis.WriteToMQ(&MQMsg{
			SenderMachID:   settings.Conf.ID,
			ReceivedMachID: machID,
			UserID:         uint64(req.UserID),
			Cmd:            CmdGrpMemChange,
			MsgType:        MsgTypeNotify,
			Data:           nil,
		}); err != nil {
			log.L().Error("Write To MQ", log.Error(err))
			continue
		}
	}
}

// GetGroupMemberList 获取群组成员列表
func GetGroupMemberList(c *gin.Context) {
	id := c.Query("group_id")
	gid, err := strconv.Atoi(id)
	if err != nil {
		c.JSON(http.StatusOK, Res(ec.ParmsInvalid, "Group Id Parse Failed"))
		log.L().Warn("Group Id Parse Failed", log.Error(err))
		return
	}
	list, err := mysql.GetGroupMemberList(gid, mysql.GrpRoleAll)
	if err != nil && !errors.Is(err, sql.ErrNoRows) {
		c.JSON(http.StatusOK, Res(ec.DBQuery, "DB Get Group Member List"))
		log.L().Error("DB Get Group Member List", log.Error(err))
		return
	}
	data, err := json.Marshal(list)
	if err != nil {
		c.JSON(http.StatusOK, Res(ec.JsonMarshal, "Group Member List Marshal Failed"))
		log.L().Error("Group Member List Marshal Failed", log.Error(err))
		return
	}
	c.JSON(http.StatusOK, OK(data))
}

// QuitGroup 退出群组
func QuitGroup(c *gin.Context) {
	var req model.QuitGroupReq
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusOK, Res(ec.BodyParseJson, "QuitGroup Parse Json Error"))
		log.L().Warn("QuitGroup Body Parse Json")
		return
	}
	// 查询用户是否在群中
	info, err := mysql.GetGroupMemberInfo(req.UserID, req.GroupID)
	if err != nil {
		c.JSON(http.StatusOK, Res(ec.DBQuery, "DB Get Group Member Info"))
		log.L().Error("DB Get Group Member Info", log.Error(err))
		return
	}
	if info.Role == mysql.GrpRoleOwner {
		c.JSON(http.StatusOK, Res(ec.PermissionDenied, "Permission Denied"))
		log.L().Warn("Quit Group Permission Denied", log.Any("request", req))
		return
	}
	// 退出群组
	if err = mysql.QuitGroup(req.UserID, req.GroupID); err != nil {
		c.JSON(http.StatusOK, Res(ec.DBModify, "DB Quit Group"))
		log.L().Error("DB Quit Group", log.Error(err))
		return
	}
	c.JSON(http.StatusOK, OK())
	// 给所有群在线成员推送消息
	members, err := mysql.GetGroupMemberList(req.GroupID, mysql.GrpRoleAll)
	if err != nil {
		log.L().Error("Get Group Member List", log.Error(err))
		return
	}
	for _, mem := range members {
		online, machID, err := redis.GetUserMachineID(strconv.Itoa(mem.UserId))
		if err != nil || !online {
			if err != nil {
				log.L().Error("Get User Machine ID", log.Error(err))
			}
			continue
		}
		if err := redis.WriteToMQ(&MQMsg{
			SenderMachID:   settings.Conf.ID,
			ReceivedMachID: machID,
			UserID:         uint64(req.UserID),
			Cmd:            CmdGrpMemChange,
			MsgType:        MsgTypeNotify,
			Data:           nil,
		}); err != nil {
			log.L().Error("Write To MQ", log.Error(err))
			continue
		}
	}
}

// ModifyGroupMemberRole 修改群成员权限
func ModifyGroupMemberRole(c *gin.Context) {
	var req model.ModifyGroupMemberRoleReq
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusOK, Res(ec.BodyParseJson, "Modify Group Member Role Parse Json Error"))
		log.L().Warn("Modify Group Member Role Body Parse Json")
		return
	}
	if req.UserID == req.EditorID {
		c.JSON(http.StatusOK, Res(ec.PermissionDenied, "Permission Denied"))
		log.L().Warn("Modify Group Member Role Permission Denied", log.Any("request", req))
		return
	}
	// 查询用户是否在群中
	memInfo, err := mysql.GetGroupMemberInfo(req.UserID, req.GroupID)
	if err != nil {
		c.JSON(http.StatusOK, Res(ec.DBQuery, "DB Get Group Member Info"))
		log.L().Error("DB Get Group Member Info", log.Error(err))
		return
	}
	// 权限验证
	if memInfo.Role == mysql.GrpRoleNormal {
		c.JSON(http.StatusOK, Res(ec.PermissionDenied, "Permission Denied"))
		log.L().Warn("Modify Group Member Role Permission Denied", log.Any("request", req))
		return
	}
	err = mysql.ModifyGroupMemberRole(req.UserID, req.GroupID, req.Role)
	if err != nil {
		c.JSON(http.StatusOK, Res(ec.DBModify, "DB Modify Group Member Role"))
		log.L().Error("DB Modify Group Member Role", log.Error(err))
		return
	}
	c.JSON(http.StatusOK, OK())
	// 推送消息
	online, machID, err := redis.GetUserMachineID(strconv.Itoa(req.UserID))
	if err != nil || !online {
		if err != nil {
			log.L().Error("Get User Machine ID", log.Error(err))
		}
		return
	}
	if err := redis.WriteToMQ(&MQMsg{
		SenderMachID:   settings.Conf.ID,
		ReceivedMachID: machID,
		UserID:         uint64(req.UserID),
		Cmd:            CmdModifyGroupMemberRole,
		MsgType:        MsgTypeNotify,
		Data:           nil,
	}); err != nil {
		log.L().Error("Write To MQ", log.Error(err))
		return
	}
}

// 成员直接入群
func addGroupNewMember(userID []int, groupID int) (err error) {
	err = mysql.AddGroupMember(userID, groupID)
	if err != nil {
		return
	}
	// 给所有群在线成员推送消息
	members, err := mysql.GetGroupMemberList(groupID, mysql.GrpRoleAll)
	if err != nil {
		log.L().Error("Get Group Member List", log.Error(err))
		return
	}
	for _, mem := range members {
		online, machID, err := redis.GetUserMachineID(strconv.Itoa(mem.UserId))
		if err != nil || !online {
			if err != nil {
				log.L().Error("Get User Machine ID", log.Error(err))
			}
			continue
		}
		if err := redis.WriteToMQ(&MQMsg{
			SenderMachID:   settings.Conf.ID,
			ReceivedMachID: machID,
			UserID:         uint64(mem.UserId),
			Cmd:            CmdGrpMemChange,
			MsgType:        MsgTypeNotify,
			Data:           nil,
		}); err != nil {
			log.L().Error("Write To MQ", log.Error(err))
			continue
		}
	}
	return
}
