package mysql

import (
	"User/log"
	"User/model"
	"User/settings"
	"database/sql"
	"fmt"
	"strconv"
	"strings"

	_ "github.com/go-sql-driver/mysql"
)

var db *sql.DB

const (
	GrpRoleAll        = -1
	GrpRoleNormal     = 0
	GrpRoleAdmin      = 1
	GrpRoleOwner      = 2
	GrpApplyPass      = 1
	GrpJoinMethAll    = 0
	GrpJoinMethApply  = 1
	GrpJoinMethInvite = 2
)

func Init() (err error) {
	dsn := fmt.Sprintf("%s:%s@tcp(%s:%d)/%s?parseTime=true&&loc=Local",
		settings.Conf.MysqlConfig.User,
		settings.Conf.MysqlConfig.Password,
		settings.Conf.MysqlConfig.Host,
		settings.Conf.MysqlConfig.Port,
		settings.Conf.MysqlConfig.DB)
	db, err = sql.Open("mysql", dsn)
	if err != nil {
		log.L().Error("Mysql Connect", log.Error(err))
		return
	}
	db.SetMaxOpenConns(settings.Conf.MysqlConfig.MaxConnections)
	db.SetMaxIdleConns(settings.Conf.MysqlConfig.MaxIdleConnections)
	return
}

func Close() {
	_ = db.Close()
}

// UserRegister 用户注册
func UserRegister(name, password, email string, gender int) (err error) {
	tx, err := db.Begin()
	if err != nil {
		if tx != nil {
			if er := tx.Rollback(); er != nil {
				log.L().Error("Mysql Transaction Rollback", log.Error(er))
			}
		}
		log.L().Error("Mysql Start Transaction", log.Error(err))
		return
	}

	// 设置盐值 TODO:使用GO完成加密
	saltSQL := "SELECT MD5(RAND())"
	var salt string
	err = tx.QueryRow(saltSQL).Scan(&salt)
	if err != nil {
		log.L().Error("DB Set Password Salt", log.Error(err))
		return
	}
	// 密码加密
	pwdSQL := fmt.Sprintf("SELECT sha2(concat('%s', '%s'), 256)", password, salt)
	var pwd string
	err = tx.QueryRow(pwdSQL).Scan(&pwd)
	if err != nil {
		log.L().Error("DB Set Password", log.Error(err))
		return
	}
	// 插入数据
	stm, err := tx.Prepare("INSERT INTO user(name, gender, password, salt, email) VALUES (?, ?, ?, ?, ?)")
	if err != nil {
		log.L().Error("DB Create Prepare", log.Error(err))
		return
	}
	defer stm.Close()

	_, err = stm.Exec(name, gender, pwd, salt, email)
	if err != nil {
		log.L().Error("DB User Insert", log.Error(err))
		return
	}
	tx.Commit()
	return
}

// UserLogin 用户登录
func UserLogin(id int, password string) (user *model.User, err error) {
	user = new(model.User)
	loginSQL := "SELECT id, name, gender, email, phone FROM user WHERE id = ? and password = SHA2(CONCAT(?, salt), 256)"
	err = db.QueryRow(loginSQL, id, password).Scan(&user.Id, &user.Name, &user.Gender, &user.Email, &user.Phone)
	return
}

// UserModify 修改用户信息
func UserModify(user *model.User) (err error) {
	updateSQL := "UPDATE user SET name = ?, gender = ?, email = ?, phone = ? WHERE id = ?"
	_, err = db.Exec(updateSQL, user.Name, user.Gender, user.Email, user.Phone, user.Id)
	if err != nil {
		log.L().Error("DB User Modify Info", log.Error(err))
	}
	return
}

// UpdateIcon 更新头像
func UpdateIcon(id int, image *model.Image) (err error) {
	SQL := "UPDATE user SET icon = ? WHERE id = ?"
	_, err = db.Exec(SQL, fmt.Sprintf("fs://%d%s", id, image.MimeType), id)
	if err != nil {
		log.L().Error("DB Update Icon", log.Error(err))
	}
	return
}

// GetIcon 获取头像
func GetIcon(id []int) (filePath []string, err error) {
	idString := make([]string, len(id))
	for i, num := range id {
		idString[i] = strconv.Itoa(num)
	}
	SQL := "SELECT icon FROM user WHERE id in (?)"
	rows, err := db.Query(SQL, strings.Join(idString, ","))
	if err != nil {
		log.L().Error("DB Query Icon", log.Error(err))
		return
	}
	defer rows.Close()
	for rows.Next() {
		var s string
		if err = rows.Scan(&s); err != nil {
			log.L().Error("DB Get Icon Rows Scan", log.Error(err))
			return
		}
		filePath = append(filePath, s)
	}
	return
}

// GetUser 获取用户信息
func GetUser(id int) (user *model.User, err error) {
	user = new(model.User)
	SQL := "SELECT id, name, gender, email, phone, icon FROM user WHERE id = ?"
	err = db.QueryRow(SQL, id).Scan(&user.Id, &user.Name, &user.Gender, &user.Email, &user.Phone, &user.Icon)
	return
}

// GetFriendship 获取好友关系
// status = 0 待处理, 1 已通过, 2 已拒绝, 3 已拉黑
func GetFriendship(id, status int) (friendList []*model.Relationship, err error) {
	stm := "SELECT id, user_id, friend_id, status, remark, group_id, alias FROM friend_relationships WHERE user_id = ? AND status = ?"
	rows, err := db.Query(stm, id, status)
	if err != nil {
		return
	}
	defer rows.Close()
	for rows.Next() {
		rel := new(model.Relationship)
		if err = rows.Scan(&rel.Id, &rel.UserID, &rel.FriendID, &rel.Status, &rel.Remark, &rel.GroupID, &rel.Alias); err != nil {
			return
		}
		friendList = append(friendList, rel)
	}
	return
}

// AddFriend 添加好友申请
func AddFriend(userId, friendId int, remark string) (id int64, err error) {
	stm := "INSERT INTO friend_relationships(user_id, friend_id, remark) VALUES (?, ?, ?)"
	res, err := db.Exec(stm, userId, friendId, remark)
	if err != nil {
		return
	}
	id, err = res.LastInsertId()
	return
}

// RespNewFriend 响应好友申请
func RespNewFriend(id int, option int) (err error) {
	stm := "UPDATE friend_relationships SET status = ? WHERE id = ?"
	_, err = db.Exec(stm, option, id)
	return
}

// AddRelGroup 添加关系分组
func AddRelGroup(userId int, name string) (id int64, err error) {
	stm := "INSERT INTO friend_groups(user_id, group_name) VALUES (?, ?)"
	res, err := db.Exec(stm, userId, name)
	if err != nil {
		return
	}
	id, err = res.LastInsertId()
	return
}

// ModifyRelGroup 修改关系分组名称
func ModifyRelGroup(gId int, name string) (err error) {
	stm := "UPDATE friend_groups SET group_name = ? WHERE id = ?"
	_, err = db.Exec(stm, name, gId)
	return
}

// ModifyFriendGroup 修改好友所属分组
func ModifyFriendGroup(id, groupId int) (err error) {
	stm := "UPDATE friend_relationships SET group_id = ? WHERE id = ?"
	_, err = db.Exec(stm, groupId, id)
	return
}

// ModifyFriendAlias 修改好友备注
func ModifyFriendAlias(id int, alias string) (err error) {
	stm := "UPDATE friend_relationships SET alias = ? WHERE id = ?"
	_, err = db.Exec(stm, alias, id)
	return
}

// NewGroup 创建群组
func NewGroup(uid int, name, description string, isPublic int) (gId int64, err error) {
	stm := "INSERT INTO groups(creator_id, name, description, public) VALUES (?, ?, ?, ?)"
	res, err := db.Exec(stm, uid, name, description, isPublic)
	if err != nil {
		return
	}
	gId, err = res.LastInsertId()
	return
}

// GetGroupInfo 获取群组信息
func GetGroupInfo(gid int) (grp *model.Group, err error) {
	grp = new(model.Group)
	stm := "SELECT id, creator_id, name, description, members_num,`public` FROM groups WHERE id = ? AND deleted = 0"
	err = db.QueryRow(stm, gid).Scan(&grp.Id, &grp.CreatorID, &grp.Name, &grp.Description, &grp.MemberCount, &grp.IsPublic)
	return
}

// GetGroupMemberInfo 获取群成员信息
func GetGroupMemberInfo(userID, groupID int) (info *model.GroupMember, err error) {
	stm := "SELECT id, user_id, group_id, role, join_at FROM group_members WHERE user_id = ? AND group_id = ? AND deleted = 0"
	info = new(model.GroupMember)
	err = db.QueryRow(stm, userID, groupID).Scan(&info.Id, &info.UserId, &info.GroupId, &info.Role, &info.JoinAt)
	return
}

// GetGroupList 获取用户群组列表
func GetGroupList(uid, role int) (grpList []*model.GroupMember, err error) {
	if role < GrpRoleAll || role > GrpRoleOwner {
		err = fmt.Errorf("invalid role")
		return
	}
	var stm string
	if role == GrpRoleAll {
		stm = "SELECT id, group_id, role, join_at FROM group_members WHERE user_id = ? AND deleted = 0"
	} else {
		stm = "SELECT id, group_id, role, join_at FROM group_members WHERE user_id = ? AND role = ? AND deleted = 0"
	}
	rows, err := db.Query(stm, uid)
	if err != nil {
		return
	}
	defer rows.Close()
	for rows.Next() {
		grp := new(model.GroupMember)
		if err = rows.Scan(&grp.Id, &grp.GroupId, &grp.Role, &grp.JoinAt); err != nil {
			return
		}
		grp.UserId = uid
		grpList = append(grpList, grp)
	}
	return
}

// DelGroup 删除群组
func DelGroup(gid int) (err error) {
	// 开启事务
	tx, err := db.Begin()
	if err != nil {
		return
	}
	defer func() {
		if err != nil {
			tx.Rollback()
		} else {
			tx.Commit()
		}
	}()
	// 设置群组表删除状态
	stm := "UPDATE groups SET deleted = 1 WHERE id = ?"
	_, err = tx.Exec(stm, gid)
	if err != nil {
		return
	}
	// 设置群设置表删除状态
	stm = "UPDATE group_settings SET deleted = 1 WHERE group_id = ?"
	_, err = tx.Exec(stm, gid)
	if err != nil {
		return
	}
	// 设置群成员表删除状态
	stm = "UPDATE group_members SET deleted = 1 WHERE group_id = ?"
	_, err = tx.Exec(stm, gid)
	if err != nil {
		return
	}
	return
}

// ApplyJoinGroup 申请加入群组
func ApplyJoinGroup(userID, groupID int) (err error) {
	stm := "INSERT INTO group_join_requests(user_id, group_id) VALUES (?, ?)"
	_, err = db.Exec(stm, userID, groupID)
	return
}

// GetGroupApplyListWithGroup 获取群申请列表
func GetGroupApplyListWithGroup(gid []int) (list []*model.GroupApply, err error) {
	stm := "SELECT id, user_id, group_id, status FROM group_join_requests WHERE status = 0 AND group_id in (?)"
	rows, err := db.Query(stm, gid)
	if err != nil {
		return
	}
	defer rows.Close()
	for rows.Next() {
		grp := new(model.GroupApply)
		if err = rows.Scan(&grp.Id, &grp.UserId, &grp.GroupId, &grp.Status); err != nil {
			return
		}
		list = append(list, grp)
	}
	return
}

// GetGroupApplyListWithUser 获取用户的群申请列表
func GetGroupApplyListWithUser(uid int) (list []*model.GroupApply, err error) {
	stm := "SELECT id, user_id, group_id, status FROM group_join_requests WHERE status = 0 AND user_id = ?"
	rows, err := db.Query(stm, uid)
	if err != nil {
		return
	}
	defer rows.Close()
	for rows.Next() {
		grp := new(model.GroupApply)
		if err = rows.Scan(&grp.Id, &grp.UserId, &grp.GroupId, &grp.Status); err != nil {
			return
		}
		list = append(list, grp)
	}
	return
}

// GroupJoinApplyDeal 群申请处理
func GroupJoinApplyDeal(applyID, userID, groupID, status int) (err error) {
	// 开启事务
	tx, err := db.Begin()
	if err != nil {
		return
	}
	defer func() {
		if err != nil {
			tx.Rollback()
		} else {
			tx.Commit()
		}
	}()
	// 设置群申请状态
	stm := "UPDATE group_join_requests SET status = ? WHERE id = ?"
	_, err = tx.Exec(stm, status, applyID)
	if err != nil {
		return
	}
	if status == GrpApplyPass {
		// 添加群成员
		stm = "INSERT INTO group_members(user_id, group_id) VALUES (?, ?)"
		_, err = tx.Exec(stm, userID, groupID)
		if err != nil {
			return
		}
		// 群人数增加
		stm = "UPDATE groups SET members_num = members_num + 1 WHERE id = ?"
		_, err = tx.Exec(stm, groupID)
	}
	return
}

// GetGroupMemberList 获取群所有成员信息
func GetGroupMemberList(gid int) (info []*model.GroupMember, err error) {
	stm := "SELECT id, user_id, group_id, role, join_at FROM group_members WHERE group_id = ? AND deleted = 0"
	rows, err := db.Query(stm, gid)
	if err != nil {
		return
	}
	defer rows.Close()
	for rows.Next() {
		grp := new(model.GroupMember)
		if err = rows.Scan(&grp.Id, &grp.UserId, &grp.GroupId, &grp.Role, &grp.JoinAt); err != nil {
			return
		}
		info = append(info, grp)
	}
	return
}

// GetGroupSetting 获取群设置
func GetGroupSetting(gid int) (setting *model.GroupSetting, err error) {
	setting = new(model.GroupSetting)
	stm := "SELECT group_id, join_method FROM group_settings WHERE group_id = ? AND deleted = 0"
	err = db.QueryRow(stm, gid).Scan(&setting.GroupId, &setting.JoinMethod)
	return
}

// AddGroupMember 成员直接入群
func AddGroupMember(userID []int, groupID int) (err error) {
	tx, err := db.Begin()
	if err != nil {
		return
	}
	defer func() {
		if err != nil {
			tx.Rollback()
		} else {
			tx.Commit()
		}
	}()
	stm := "INSERT INTO group_members(user_id, group_id) VALUES (?, ?)"
	for _, v := range userID {
		_, err = tx.Exec(stm, v, groupID)
		if err != nil {
			return
		}
	}
	stm = "UPDATE groups SET members_num = members_num + ? WHERE id = ?"
	_, err = tx.Exec(stm, len(userID), groupID)
	return
}

// QuitGroup 退出群组
func QuitGroup(userID, groupID int) (err error) {
	tx, err := db.Begin()
	if err != nil {
		return
	}
	defer func() {
		if err != nil {
			tx.Rollback()
		} else {
			tx.Commit()
		}
	}()
	stm := "UPDATE group_members SET deleted = 1 WHERE user_id = ? AND group_id = ?"
	_, err = tx.Exec(stm, userID, groupID)
	if err != nil {
		return
	}
	stm = "UPDATE groups SET members_num = members_num - 1 WHERE id = ?"
	_, err = tx.Exec(stm, groupID)
	return
}

// ModifyGroupMemberRole 修改成员权限
func ModifyGroupMemberRole(userID, groupID, role int) (err error) {
	stm := "UPDATE group_members SET role = ? WHERE user_id = ? AND group_id = ? AND deleted = 0"
	_, err = db.Exec(stm, role, userID, groupID)
	return
}
