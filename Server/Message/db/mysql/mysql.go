package mysql

import (
	"Message/log"
	"Message/protocol"
	"Message/utils"

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

func SaveMessage(msg *protocol.Msg) (err error) {
	msgID, err := utils.GenID(settings.Conf.MachineID)
	if err != nil {
		return
	}
	_, err = db.Exec("INSERT INTO messages(id, sender_id, receiver_id, msg_type,msg_content) VALUES (?, ?, ?, ?, ?)",
		msgID, msg.SenderId, msg.ReceiverId, msg.ContentType, msg.Content)
	return
}

func UpdateConversation(msg *protocol.Msg) (err error) {
	_, err = db.Exec("UPDATE conversations SET last_ack_msg_id = ?, WHERE `id` = ?", msg.ConversationId, msg.ConversationId)
	return
}
