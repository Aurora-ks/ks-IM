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

func Init() (err error) {
	dsn := fmt.Sprintf("%s:%s@tcp(%s:%d)/%s",
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

func UserLogin(id int, password string) (user *model.User, err error) {
	user = new(model.User)
	loginSQL := "SELECT id, name, gender, email, phone FROM user WHERE id = ? and password = SHA2(CONCAT(?, salt), 256)"
	err = db.QueryRow(loginSQL, id, password).Scan(&user.Id, &user.Name, &user.Gender, &user.Email, &user.Phone)
	return
}

func UserModify(user *model.User) (err error) {
	updateSQL := "UPDATE user SET name = ?, gender = ?, email = ?, phone = ? WHERE id = ?"
	_, err = db.Exec(updateSQL, user.Name, user.Gender, user.Email, user.Phone, user.Id)
	if err != nil {
		log.L().Error("DB User Modify Info", log.Error(err))
	}
	return
}

func UpdateIcon(id int, image *model.Image) (err error) {
	SQL := "UPDATE user SET icon = ? WHERE id = ?"
	_, err = db.Exec(SQL, fmt.Sprintf("fs://%d%s", id, image.MimeType), id)
	if err != nil {
		log.L().Error("DB Update Icon", log.Error(err))
	}
	return
}

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

func GetUser(id int) (user *model.User, err error) {
	user = new(model.User)
	SQL := "SELECT id, name, gender, email, phone, icon FROM user WHERE id = ?"
	err = db.QueryRow(SQL, id).Scan(&user.Id, &user.Name, &user.Gender, &user.Email, &user.Phone, &user.Icon)
	return
}

func GetFriendLists(id int) (friendList []*model.Relationship, err error) {
	stm := "SELECT id, user_id, friend_id, status, remark, group_id FROM friend_relationships WHERE user_id = ? AND status = 1"
	rows, err := db.Query(stm, id)
	if err != nil {
		return
	}
	defer rows.Close()
	for rows.Next() {
		rel := new(model.Relationship)
		if err = rows.Scan(&rel.Id, &rel.UserID, &rel.FriendID, &rel.Status, &rel.Remark, &rel.GroupID); err != nil {
			return
		}
		friendList = append(friendList, rel)
	}
	return
}
func AddFriend(userId, friendId int, remark string) (id int64, err error) {
	stm := "INSERT INTO friend_relationships(user_id, friend_id, remark) VALUES (?, ?, ?)"
	res, err := db.Exec(stm, userId, friendId, remark)
	if err != nil {
		return
	}
	id, err = res.LastInsertId()
	return
}
func RespNewFriend(id int, option int) (err error) {
	stm := "UPDATE friend_relationships SET status = ? WHERE id = ?"
	_, err = db.Exec(stm, option, id)
	return
}
