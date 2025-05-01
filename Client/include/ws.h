#ifndef WS_H
#define WS_H

#include <QObject>
#include <QUrl>
#include <QTimer>
#include <QThread>
#include <QMutex>
#include <QQueue>
#include <QWaitCondition>
#include <QMap>
#include <QDateTime>
#include <memory>
#include "net.h"
#include "protocol.pb.h"
#include "logger.h"
#include "user.h"

// 命令类型常量
enum CmdType {
    CMD_ACK = 0,              // ack
    CMD_SC = 1,               // 单聊
    CMD_GC = 2,               // 群聊
    CMD_MSG_DEL_S = 3,        // 单聊消息删除
    CMD_GRP_APPLY = 4,        // 群组加入申请
    CMD_GRP_APPLY_RESP = 5,   // 群组申请响应
    CMD_GRP_MEM_CHANGE = 6,   // 群成员变动
    CMD_MODIFY_GRP_MEM_ROLE = 7, // 修改群成员角色
    CMD_REL_APPLY = 8         // 好友申请
};

// 消息类型常量
enum MsgType {
    MSG_TYPE_REQUEST = 0,    // 请求
    MSG_TYPE_RESPONSE = 1,   // 响应
    MSG_TYPE_NOTIFY = 2,     // 通知
    MSG_TYPE_PUSH = 3,       // 推送
    MSG_TYPE_ERROR = 4,      // 错误
    MSG_TYPE_ACK = 5         // 确认
};

#define WsIns WebSocketManager::getInstance()
class WebSocketManager : public QObject
{
    Q_OBJECT

public:
    static WebSocketManager& getInstance() { static WebSocketManager instance; return instance; }
    ~WebSocketManager();

    // 连接相关方法
    bool connectToServer(const QString& url);
    void disconnectFromServer();
    bool isConnected() const;

    // 消息发送方法
    uint64_t sendSingleChatMessage(uint64_t conversationId, uint64_t receiverId,
                     const QByteArray& content, uint32_t contentType, const QString& fileName = QString());
    bool sendGroupChatMessage(uint64_t conversationId, uint64_t groupId,
                     const QByteArray& content, uint32_t contentType);
    bool sendAck(uint64_t seq, uint64_t convId = 0, uint64_t lastMsgId = 0);

    // 序列号生成
    uint64_t generateSeq();

signals:
    // 连接状态信号
    void connected();
    void disconnected();
    void connectionError(QAbstractSocket::SocketError error, const QString& errorString);

    // 消息接收信号
    void messageReceived(uint64_t seq, const protocol::Msg& message);
    void ackReceived(const protocol::MsgACK_S& ack);
    void groupNotifyReceived(const protocol::GroupMsgNotify& notify);
    void relationApplyReceived(uint64_t applyId);
    void messageIdReceived(uint64_t seq, uint64_t sessionId, uint64_t messageId);

private:
    explicit WebSocketManager(QObject* parent = nullptr);
    WebSocketManager(const WebSocketManager&) = delete;
    WebSocketManager& operator=(const WebSocketManager&) = delete;

    void init();
    void cleanup();
    QString socketErrorToString(QAbstractSocket::SocketError error);

    // 回调处理方法
    void handleConnected();
    void handleDisconnected();
    void handleBinaryMessage(const QByteArray& data);
    void handleError(QAbstractSocket::SocketError error);

    // 消息处理方法
    void processPacket(const protocol::Packet& packet);

    // WebSocket相关
    Net* ws_;
    QUrl serverUrl_;
    bool connected_;

    // 消息相关
    uint64_t seqCounter_;
    QMutex seqMutex_;

    // 消息重发相关
    struct PendingMessage {
        QByteArray data;
        uint64_t seq;
        int retryCount;
        QTimer* retryTimer;  // 每个消息独立的计时器
    };
    QMap<uint64_t, PendingMessage> pendingMessages_;  // 待确认消息队列
    QMutex pendingMessagesMutex_;  // 待确认消息队列互斥锁
    int retryInterval_;  // 重发间隔(毫秒)
    int maxRetryCount_;  // 最大重发次数

    // 消息重发相关方法
    void addPendingMessage(const QByteArray& data, uint64_t seq);
    void removePendingMessage(uint64_t seq);
    void onRetryTimeout(uint64_t seq);
};

#endif // WS_H
