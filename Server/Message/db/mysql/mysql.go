package mysql

import (
	"Message/log"
	"Message/protocol"
	//"Message/model"
	"Message/settings"
	"database/sql"
	"fmt"
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

// SaveMessage 存储新消息
func SaveMessage(msg *protocol.Msg, isGroup bool) (err error) {
	// TODO: add file name
	isGrp := 0
	if isGroup {
		isGrp = 1
	}
	if msg.ContentType == 0 {
		stm := "INSERT INTO messages(id, sender_id, receiver_id, msg_type, msg_content, is_group) VALUES (?, ?, ?, ?, ?, ?)"
		_, err = db.Exec(stm, msg.MsgId, msg.SenderId, msg.ReceiverId, msg.ContentType, msg.Content, isGrp)
	} else {
		stm := "INSERT INTO messages(id, sender_id, receiver_id, msg_type, is_group, file_name, msg_file) VALUES (?, ?, ?, ?, ?, ?, ?)"
		_, err = db.Exec(stm, msg.MsgId, msg.SenderId, msg.ReceiverId, msg.ContentType, isGrp, msg.FileName, msg.Content)
	}
	return
}
func UpdateConversation(cid, uid, mid uint64) (err error) {
	stm := `UPDATE conversations 
			SET 
				u1_last_ack_msg = IF(uid1 = ?, ?, u1_last_ack_msg),
				u2_last_ack_msg = IF(uid2 = ?, ?, u2_last_ack_msg) 
			WHERE id = ?`
	_, err = db.Exec(stm, uid, mid, uid, mid, cid)
	return
}
func CreateNewConversation(uid uint64, pid uint64, isGroup bool) (cid uint64, err error) {
	grp := 0
	if isGroup {
		grp = 1
	}

	stm := "INSERT INTO conversations(uid1, uid2, is_group) VALUES (?, ?, ?)"
	res, err := db.Exec(stm, uid, pid, grp)
	if err != nil {
		return
	}
	id, err := res.LastInsertId()
	cid = uint64(id)
	return
}
func GetConversationID(uid, peerID uint64) (cid uint64, err error) {
	stm := "SELECT id FROM conversations WHERE (uid1 = ? AND uid2 = ?) OR (uid2 = ? AND uid1 = ?) AND deleted = 0"
	err = db.QueryRow(stm, uid, peerID, uid, peerID).Scan(&cid)
	return
}
func DelMsg(mid uint64) (err error) {
	stm := "DELETE FROM messages WHERE id = ?"
	_, err = db.Exec(stm, mid)
	return
}
func GetGroupMembersID(gid uint64) (m []uint64, err error) {
	stm := "SELECT user_id FROM group_members WHERE group_id = ? AND deleted = 0"
	rows, err := db.Query(stm, gid)
	if err != nil {
		return
	}
	defer rows.Close()
	for rows.Next() {
		var id uint64
		err = rows.Scan(&id)
		if err != nil {
			return
		}
		m = append(m, id)
	}
	return
}
