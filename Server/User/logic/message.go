package logic

import (
	"User/db/mysql"
	"User/ec"
	"User/log"
	"database/sql"
	"encoding/json"
	"errors"
	"net/http"
	"strconv"

	"github.com/gin-gonic/gin"
)

// GetOfflineMsg 获取离线消息
func GetOfflineMsg(c *gin.Context) {
	uid := c.Query("uid")
	userId, err := strconv.Atoi(uid)
	if err != nil {
		c.JSON(http.StatusOK, Res(ec.ParmsInvalid, "User Id Parse Failed"))
		log.L().Error("User Id Parse Failed", log.Error(err))
		return
	}

	ack := c.Query("last_ack")
	lastAckId, err := strconv.Atoi(ack)
	if err != nil {
		c.JSON(http.StatusOK, Res(ec.ParmsInvalid, "Ack Id Parse Failed"))
		log.L().Error("Ack Id Parse Failed", log.Error(err))
		return
	}

	sender := c.Query("sender_id")
	senderId, err := strconv.Atoi(sender)
	if err != nil {
		c.JSON(http.StatusOK, Res(ec.ParmsInvalid, "Sender Id Parse Failed"))
		log.L().Error("Sender Id Parse Failed", log.Error(err))
		return
	}

	sid := c.Query("session_id")
	sessionId, err := strconv.Atoi(sid)
	if err != nil {
		c.JSON(http.StatusOK, Res(ec.ParmsInvalid, "Session Id Parse Failed"))
		log.L().Error("Session Id Parse Failed", log.Error(err))
		return
	}

	list, err := mysql.GetOfflineMsg(sessionId, userId, senderId, lastAckId)
	if err != nil && !errors.Is(err, sql.ErrNoRows) {
		log.L().Error("DB Get Offline Msg", log.Error(err))
		c.JSON(http.StatusOK, Res(ec.DBQuery, "DB Get Offline Msg"))
		return
	}
	data, err := json.Marshal(list)
	if err != nil {
		c.JSON(http.StatusOK, Res(ec.JsonMarshal, "Offline Msg Marshal Failed"))
		log.L().Error("Offline Msg Marshal Failed", log.Error(err))
		return
	}
	c.JSON(http.StatusOK, OK(data))
}
