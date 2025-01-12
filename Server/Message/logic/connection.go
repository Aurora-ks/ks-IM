package logic

import (
	"Message/db/redis"
	"Message/log"
	"Message/protocol"
	"github.com/gin-gonic/gin"
	"github.com/gorilla/websocket"
	"net/http"
	"sync"
	"time"
)

var wsUpgrader = websocket.Upgrader{
	ReadBufferSize:   1024,
	WriteBufferSize:  1024,
	HandshakeTimeout: 5 * time.Second,
	// 取消ws跨域校验
	CheckOrigin: func(r *http.Request) bool {
		return true
	},
}
var connectionsMap sync.Map // uid[int]-con[*Connection]
var ackQueueMap sync.Map    // seq[int]-item[*DataItem]

const (
	pingInterval  = 30 * time.Second
	retryInterval = 10 * time.Second
	retryCount    = 3
)

type Connection struct {
	Conn        *websocket.Conn
	Uid         string
	ReceiveChan chan []byte
	SendChan    chan []byte
}

type DataItem struct {
	Seq       uint64
	Data      []byte
	Timer     *time.Timer
	Retries   int
	ACKHandle func(params ...any)
}

func (con *Connection) Send(seq uint64, cmd, msgType uint32, data []byte) {
	p, err := protocol.Encode(seq, cmd, msgType, data)
	if err != nil {
		log.L().Error("Encode Message", log.Error(err), log.String("user_id", con.Uid))
		return
	}
	con.SendChan <- p
}

func (con *Connection) SendRaw(data []byte) {
	con.SendChan <- data
}

func (con *Connection) SendWithACK(seq uint64, cmd, msgType uint32, data []byte, ackHandler func(params ...any)) {
	p, err := protocol.Encode(seq, cmd, msgType, data)
	if err != nil {
		log.L().Error("Encode Message", log.Error(err), log.String("user_id", con.Uid))
		return
	}
	con.SendChan <- p

	item := &DataItem{
		Seq:       seq,
		Data:      p,
		Retries:   0,
		Timer:     time.AfterFunc(retryInterval, func() { con.SendRetry(seq) }),
		ACKHandle: ackHandler,
	}
	ackQueueMap.Store(seq, item)
}

func (con *Connection) SendACK(seq uint64, cmd uint32) {
	con.Send(seq, cmd, MsgTypeACK, []byte{})
}

func (con *Connection) SendError(seq uint64, cmd uint32) {
	con.Send(seq, cmd, MsgTypeError, []byte{})
}

func (con *Connection) SendRetry(seq uint64) {
	i, exist := ackQueueMap.Load(seq)
	if !exist {
		return
	}
	item := i.(*DataItem)
	// 重传次数过多
	if item.Retries >= retryCount {
		log.L().Debug("Retry Count Exceeded", log.String("user_id", con.Uid), log.Uint64("seq", seq))
		ackQueueMap.Delete(seq)
		return
	}
	// 重传
	con.SendRaw(item.Data)
	item.Retries++
	item.Timer.Reset(retryInterval)
}
func NewConnection(context *gin.Context) {
	uid := context.Param("id")
	client, err := wsUpgrader.Upgrade(context.Writer, context.Request, nil)
	if err != nil {
		log.L().Error("Websocket Upgrade", log.Error(err), log.String("user_id", uid))
		client.Close()
		return
	}

	connection := &Connection{
		Uid:         uid,
		Conn:        client,
		ReceiveChan: make(chan []byte, 100),
		SendChan:    make(chan []byte, 100),
	}
	err = addConnection(connection)
	if err != nil {
		log.L().Error("Add Connection", log.Error(err), log.String("user_id", uid))
		client.Close()
		return
	}

	client.SetCloseHandler(func(code int, text string) error {
		if err = removeConnection(uid); err != nil {
			log.L().Error("Remove Connection", log.Error(err), log.String("user_id", uid))
		}
		return nil
	})

	connHandler(connection)
}

func addConnection(con *Connection) error {
	uid := con.Uid
	conn := con.Conn
	connectionsMap.Store(uid, conn)
	return redis.UserOnline(uid)
}

func removeConnection(uid string) error {
	connectionsMap.Delete(uid)
	return redis.UserOffline(uid)
}

func connHandler(con *Connection) {
	defer con.Conn.Close()

	quit := make(chan bool)
	go heartbeat(con, quit)
	go readMessage(con, quit)

	for {
		select {
		// 收到消息
		case data := <-con.ReceiveChan:
			go dispatchMessage(data, con)
		// 发送消息
		case data := <-con.SendChan:
			err := con.Conn.WriteMessage(websocket.TextMessage, data)
			if err != nil {
				log.L().Error("Write Message", log.Error(err), log.String("user_id", con.Uid))
				quit <- true
				return
			}
		// 退出
		case <-quit:
			return
		}
	}
}

func readMessage(con *Connection, quit chan bool) {
	for {
		_, p, err := con.Conn.ReadMessage()
		if err != nil {
			if !isNormalClose(err) {
				log.L().Error("Read Message", log.Error(err), log.String("user_id", con.Uid))
			}
			quit <- true
			return
		}
		con.ReceiveChan <- p
	}
}

func dispatchMessage(data []byte, con *Connection) {
	p, err := protocol.Decode(data)
	if err != nil {
		log.L().Error("Decode Message", log.Error(err), log.String("user_id", con.Uid))
		return
	}
	if int(p.DataLength) != len(p.Data) {
		con.SendError(p.Seq, p.Cmd)
		log.L().Error("Data Length Error", log.Int("data_length", int(p.DataLength)), log.String("user_id", con.Uid))
		return
	}

	switch p.Cmd {
	case CmdACK:
		handlACK(p.Seq)
	case CmdSC:
		SingleChatHandler(con, p)
	case CmdGC:
	default:
		con.SendError(p.Seq, p.Cmd)
		log.L().Error("Unknown Cmd", log.Int("cmd", int(p.Cmd)), log.String("user_id", con.Uid))
	}
}

func handlACK(seq uint64) {
	i, exists := ackQueueMap.Load(seq)
	if !exists {
		return
	}
	item := i.(*DataItem)
	if item.ACKHandle != nil {
		item.ACKHandle()
	}
	ackQueueMap.Delete(seq)
	log.L().Debug("ACK Received", log.Uint64("seq", seq))
}

func heartbeat(con *Connection, quit chan bool) {
	// 启动心跳发送定时器
	ticker := time.NewTicker(pingInterval)
	defer ticker.Stop()

	retries := 0
	con.Conn.SetPongHandler(func(string) error {
		retries = 0
		return nil
	})

	for {
		select {
		// 心跳发送
		case <-ticker.C:
			if retries > retryCount {
				log.L().Debug("Heartbeat timeout after retries", log.String("user_id", con.Uid))
				quit <- true
				return
			}
			if err := con.Conn.WriteMessage(websocket.PingMessage, []byte{}); err != nil {
				log.L().Error("Ping Write", log.Error(err), log.String("user_id", con.Uid))
				quit <- true
				return
			}
			retries++
			log.L().Debug("Heartbeat sent", log.String("user_id", con.Uid), log.Int("retries", retries))
		case <-quit:
			return
		}
	}
}

// 判断连接是否是正常退出
func isNormalClose(err error) bool {
	if websocket.IsCloseError(err, websocket.CloseNormalClosure, websocket.CloseGoingAway, websocket.CloseNoStatusReceived) {
		return true
	}
	return false
}
