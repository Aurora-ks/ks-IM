#include "ws.h"
#include <QDateTime>
#include "logger.h"
#include "user.h"

WebSocketManager::WebSocketManager(QObject *parent)
        : QObject(parent), ws_(new Net(NetType::WS)), connected_(false),
          seqCounter_(0), retryInterval_(5000)  // 5秒重发间隔
        , maxRetryCount_(3) {  // 最大重发3次
    init();
}

WebSocketManager::~WebSocketManager() {
    cleanup();
}

void WebSocketManager::init() {
    // 设置WebSocket回调
    ws_->setWsConnectedCallback([this]() { handleConnected(); });
    ws_->setWsDisconnectedCallback([this]() { handleDisconnected(); });
    ws_->setWsBinaryCallback([this](const QByteArray &data) { handleBinaryMessage(data); });
    ws_->setWsErrorCallback([this](QAbstractSocket::SocketError error) { handleError(error); });
    ws_->setWsPongCallback([](quint64 elapsedTime, const QByteArray &payload) {
        LOG_DEBUG("[Net] [Pong] [uid:{}] {}", User::GetUid(), payload.toStdString());
    });
}

void WebSocketManager::cleanup() {
    // 清理所有待确认消息的计时器
    {
        QMutexLocker locker(&pendingMessagesMutex_);
        for (auto it = pendingMessages_.begin(); it != pendingMessages_.end(); ++it) {
            if (it->retryTimer) {
                it->retryTimer->stop();
                delete it->retryTimer;
            }
        }
        pendingMessages_.clear();
    }

    disconnectFromServer();
}

bool WebSocketManager::connectToServer(const QString &url) {
    bool success = ws_->connectToUrl(url);
    if (success) {
        LOG_INFO("[Net] [WebSocketManager::connectToServer] [uid:{}] ws连接成功", User::GetUid());
        connected_ = true;
        emit connected();
    } else {
        LOG_ERROR("[Net] [WebSocketManager::connectToServer] [uid:{}] ws连接失败", User::GetUid());
    }
    return success;
}

void WebSocketManager::disconnectFromServer() {
    if (ws_) {
        ws_->disconnect();
        connected_ = false;
    }
    delete ws_;
    ws_ = nullptr;
}

bool WebSocketManager::isConnected() const {
    return connected_;
}

uint64_t WebSocketManager::sendSingleChatMessage(uint64_t conversationId, uint64_t receiverId,
                                                 const QByteArray &content, uint32_t contentType,
                                                 const QString &fileName) {
    if (!ws_ || !connected_) {
        LOG_ERROR("[Net] [WebSocketManager::sendSingleChatMessage] [uid:{}] 尝试在未连接状态下发送消息",
                  User::GetUid());
        return -1;
    }

    // 创建消息
    protocol::Msg msg;
    msg.set_conversation_id(conversationId);
    msg.set_sender_id(User::GetUid());
    msg.set_receiver_id(receiverId);
    msg.set_content_type(contentType);
    msg.set_is_group(false);
    msg.set_content(content.data(), content.size());
    if (!fileName.isEmpty()) {
        msg.set_file_name(fileName.toStdString());
    }
    // 计算序列化后的大小
    size_t msgSize = msg.ByteSizeLong();
    std::string msgSerialized;
    msgSerialized.resize(msgSize);
    if (!msg.SerializeToArray(msgSerialized.data(), static_cast<int>(msgSize))) {
        LOG_ERROR("[Net] [WebSocketManager::sendSingleChatMessage] [uid:{}] Msg序列化失败", User::GetUid());
        return -1;
    }

    // 创建数据包
    protocol::Packet packet;
    uint64_t seq = generateSeq();
    packet.set_seq(seq);
    packet.set_timestamp(QDateTime::currentMSecsSinceEpoch());
    packet.set_cmd(CMD_SC);  // 单聊命令
    packet.set_msg_type(MSG_TYPE_REQUEST);  // 请求类型

    // 设置数据内容
    packet.set_data(msgSerialized.data(), msgSerialized.size());
    packet.set_data_length(static_cast<uint32_t>(msgSerialized.size()));

    // 计算Packet序列化后的大小
    size_t packetSize = packet.ByteSizeLong();
    std::string packetSerialized;
    packetSerialized.resize(packetSize);
    if (!packet.SerializeToArray(packetSerialized.data(), static_cast<int>(packetSize))) {
        LOG_ERROR("[Net] [WebSocketManager::sendSingleChatMessage] [uid:{}] Packet序列化失败", User::GetUid());
        return -1;
    }

    // 直接发送二进制数据
    QByteArray data(packetSerialized.data(), static_cast<int>(packetSize));
    ws_->sendWsBinary(data);

    // 将消息加入待确认队列
    addPendingMessage(data, seq);
    return seq;
}

bool WebSocketManager::sendGroupChatMessage(uint64_t conversationId, uint64_t groupId,
                                            const QByteArray &content, uint32_t contentType) {
    if (!ws_ || !connected_) {
        LOG_ERROR("[Net] [WebSocketManager::sendGroupChatMessage] [uid:{}] 尝试在未连接状态下发送消息", User::GetUid());
        return false;
    }

    // 创建消息
    protocol::Msg msg;
    msg.set_conversation_id(conversationId);
    msg.set_sender_id(User::GetUid());
    msg.set_receiver_id(groupId);  // 群ID作为接收者
    msg.set_content_type(contentType);
    msg.set_is_group(true);
    msg.set_content(content.data(), content.size());

    // 计算序列化后的大小
    size_t msgSize = msg.ByteSizeLong();
    std::string msgSerialized;
    msgSerialized.resize(msgSize);
    if (!msg.SerializeToArray(msgSerialized.data(), static_cast<int>(msgSize))) {
        LOG_ERROR("[Net] [WebSocketManager::sendGroupChatMessage] [uid:{}] Msg序列化失败", User::GetUid());
        return false;
    }

    // 创建数据包
    protocol::Packet packet;
    uint64_t seq = generateSeq();
    packet.set_seq(seq);
    packet.set_timestamp(QDateTime::currentMSecsSinceEpoch());
    packet.set_cmd(CMD_GC);  // 群聊命令
    packet.set_msg_type(MSG_TYPE_REQUEST);  // 请求类型

    // 设置数据内容
    packet.set_data(msgSerialized.data(), msgSerialized.size());
    packet.set_data_length(static_cast<uint32_t>(msgSerialized.size()));

    // 计算Packet序列化后的大小
    size_t packetSize = packet.ByteSizeLong();
    std::string packetSerialized;
    packetSerialized.resize(packetSize);
    if (!packet.SerializeToArray(packetSerialized.data(), static_cast<int>(packetSize))) {
        LOG_ERROR("[Net] [WebSocketManager::sendGroupChatMessage] [uid:{}] Packet序列化失败", User::GetUid());
        return false;
    }

    // 直接发送二进制数据
    QByteArray data(packetSerialized.data(), static_cast<int>(packetSize));
    ws_->sendWsBinary(data);

    // 将消息加入待确认队列
    addPendingMessage(data, seq);

    LOG_INFO("[Net] [WebSocketManager::sendGroupChatMessage] [uid:{}] 群聊消息已发送，群: {}, 会话ID: {}",
             User::GetUid(), groupId,
             conversationId);
    return true;
}

bool WebSocketManager::sendAck(uint64_t seq, uint64_t convId, uint64_t lastMsgId) {
    if (!ws_ || !connected_) {
        LOG_ERROR("[Net] [WebSocketManager::sendAck] [uid:{}] 尝试在未连接状态下发送ACK", User::GetUid());
        return false;
    }

    // 创建ACK
    protocol::MsgACK_C ack;
    ack.set_seq(seq);

    if (convId > 0) {
        ack.set_conv_id(convId);
    }

    if (lastMsgId > 0) {
        ack.set_last_msg_id(lastMsgId);
    }

    // 计算序列化后的大小
    size_t ackSize = ack.ByteSizeLong();
    std::string ackSerialized;
    ackSerialized.resize(ackSize);
    if (!ack.SerializeToArray(ackSerialized.data(), static_cast<int>(ackSize))) {
        LOG_ERROR("[Net] [WebSocketManager::sendAck] [uid:{}] MsgACK_C序列化失败", User::GetUid());
        return false;
    }

    // 创建数据包
    protocol::Packet packet;
    packet.set_seq(generateSeq());
    packet.set_timestamp(QDateTime::currentMSecsSinceEpoch());
    packet.set_cmd(CMD_SC);  // ACK命令
    packet.set_msg_type(MSG_TYPE_ACK);  // ACK类型

    // 设置数据内容
    packet.set_data(ackSerialized.data(), ackSerialized.size());
    packet.set_data_length(static_cast<uint32_t>(ackSerialized.size()));

    // 计算Packet序列化后的大小
    size_t packetSize = packet.ByteSizeLong();
    std::string packetSerialized;
    packetSerialized.resize(packetSize);
    if (!packet.SerializeToArray(packetSerialized.data(), static_cast<int>(packetSize))) {
        LOG_ERROR("[Net] [WebSocketManager::sendAck] [uid:{}] Packet序列化失败", User::GetUid());
        return false;
    }

    // 直接发送二进制数据
    QByteArray data(packetSerialized.data(), static_cast<int>(packetSize));
    ws_->sendWsBinary(data);

    LOG_DEBUG("[Net] [WebSocketManager::sendAck] [uid:{}] ACK已发送，seq: {}, convId: {}, lastMsgId: {}",
              User::GetUid(), seq, convId, lastMsgId);
    return true;
}

uint64_t WebSocketManager::generateSeq() {
    QMutexLocker locker(&seqMutex_);
    return ++seqCounter_;
}

void WebSocketManager::handleConnected() {
    connected_ = true;
    emit connected();
}

void WebSocketManager::handleDisconnected() {
    connected_ = false;
    // 停止所有重发计时器
    {
        QMutexLocker locker(&pendingMessagesMutex_);
        for (auto it = pendingMessages_.begin(); it != pendingMessages_.end(); ++it) {
            if (it->retryTimer) {
                it->retryTimer->stop();
            }
        }
    }
    emit disconnected();
    LOG_INFO("[Net] [WebSocketManager::handleDisconnected] [uid:{}] ws断开，当前待确认消息数: {}",
             User::GetUid(),
             std::to_string(pendingMessages_.size()));
}

void WebSocketManager::handleBinaryMessage(const QByteArray &data) {
    protocol::Packet packet;
    if (!packet.ParseFromArray(data.data(), data.size())) {
        LOG_ERROR("[Net] [WebSocketManager::handleBinaryMessage] [uid:{}] 收到的WebSocket消息无法解析为Packet",
                  User::GetUid());
        return;
    }
    processPacket(packet);
}

void WebSocketManager::handleError(QAbstractSocket::SocketError error) {
    QString errorString = socketErrorToString(error);
    LOG_ERROR("[Net] [WebSocketManager::handleError] [uid:{}] ws错误: {}, 错误码: {}",
              User::GetUid(),
              errorString.toStdString(),
              socketErrorToString(error).toStdString());
    emit connectionError(error, errorString);
}

void WebSocketManager::processPacket(const protocol::Packet &packet) {
    uint32_t cmd = packet.cmd();
    uint32_t msgType = packet.msg_type();

    LOG_DEBUG("[Net] [WebSocketManager::processPacket] [uid:{}] 收到消息: cmd={}, msgType={}, seq={}",
              User::GetUid(), cmd, msgType, packet.seq());

    // 处理数据包内容
    switch (cmd) {
        case CMD_SC:  // 单聊消息
        case CMD_GC:  // 群聊消息
        {
            if (msgType != MSG_TYPE_NOTIFY) return;
            protocol::Msg msg;
            if (msg.ParseFromArray(packet.data().data(), packet.data().size())) {
                emit messageReceived(packet.seq(), msg);

                // 发送ACK确认收到
                sendAck(packet.seq(), msg.conversation_id(), msg.msg_id());
            }
            break;
        }
        case CMD_ACK:  // 服务器ACK
        {
            if (msgType != MSG_TYPE_ACK) return;
            protocol::MsgACK_S ack;
            if (ack.ParseFromArray(packet.data().data(), packet.data().size())) {
                // 更新消息ID
                if (ack.has_msg_id()) {
                    // 从待确认队列中获取会话ID
                    uint64_t sessionId = 0;
                    {
                        QMutexLocker locker(&pendingMessagesMutex_);
                        auto it = pendingMessages_.find(ack.seq());
                        if (it != pendingMessages_.end()) {
                            protocol::Packet pendingPacket;
                            if (pendingPacket.ParseFromArray(it->data.data(), it->data.size())) {
                                protocol::Msg msg;
                                if (msg.ParseFromArray(pendingPacket.data().data(), pendingPacket.data().size())) {
                                    sessionId = msg.conversation_id();
                                }
                            }
                        }
                    }
                    emit messageIdReceived(ack.seq(), sessionId, ack.msg_id());
                }
                // 收到ACK后，从待确认队列中移除对应的消息
                removePendingMessage(ack.seq());
                emit ackReceived(ack);
            }
            break;
        }
        case CMD_REL_APPLY:  // 好友申请
        {
            // 解析好友申请消息
            // 这里简化处理，实际应根据具体协议解析
            uint64_t applyId = 0;
            // 从数据中解析申请ID
            emit relationApplyReceived(applyId);
            break;
        }
        default:
            LOG_WARN("[Net] [WebSocketManager::processPacket] [uid:{}] 收到未知类型的消息: cmd={}, msgType={}",
                     User::GetUid(), cmd, msgType);
            break;
    }
}

void WebSocketManager::addPendingMessage(const QByteArray &data, uint64_t seq) {
    QMutexLocker locker(&pendingMessagesMutex_);
    PendingMessage msg;
    msg.data = data;
    msg.seq = seq;
    msg.retryCount = 0;

    // 创建独立的计时器
    msg.retryTimer = new QTimer(this);
    msg.retryTimer->setInterval(retryInterval_);
    connect(msg.retryTimer, &QTimer::timeout, this, [this, seq]() { onRetryTimeout(seq); });
    msg.retryTimer->start();

    pendingMessages_[seq] = msg;
}

void WebSocketManager::removePendingMessage(uint64_t seq) {
    QMutexLocker locker(&pendingMessagesMutex_);
    auto it = pendingMessages_.find(seq);
    if (it != pendingMessages_.end()) {
        if (it->retryTimer) {
            it->retryTimer->stop();
            delete it->retryTimer;
        }
        pendingMessages_.erase(it);
    }
}

void WebSocketManager::onRetryTimeout(uint64_t seq) {
    if (!connected_) {
        LOG_WARN("[Net] [WebSocketManager::onRetryTimeout] [uid:{}] WebSocket未连接，取消重发消息，seq: {}",
                 User::GetUid(),
                 seq);
        return;
    }

    QMutexLocker locker(&pendingMessagesMutex_);
    auto it = pendingMessages_.find(seq);
    if (it == pendingMessages_.end()) return;

    PendingMessage &msg = it.value();
    if (msg.retryCount >= maxRetryCount_) {
        LOG_WARN("[Net] [WebSocketManager::onRetryTimeout] [uid:{}] 消息重发次数超过上限，seq: {}",
                 User::GetUid(),
                 seq);
        if (msg.retryTimer) {
            msg.retryTimer->stop();
            delete msg.retryTimer;
        }
        pendingMessages_.erase(it);
        return;
    }

    msg.retryCount++;
    LOG_DEBUG("[Net] [WebSocketManager::onRetryTimeout] [uid:{}] 重发消息，seq: {}, 重发次数: {}",
              User::GetUid(),
              seq,
              msg.retryCount);
    ws_->sendWsBinary(msg.data);
}

QString WebSocketManager::socketErrorToString(QAbstractSocket::SocketError error) {
    switch (error) {
        case QAbstractSocket::ConnectionRefusedError:
            return "ConnectionRefusedError";
        case QAbstractSocket::RemoteHostClosedError:
            return "RemoteHostClosedError";
        case QAbstractSocket::HostNotFoundError:
            return "HostNotFoundError";
        case QAbstractSocket::SocketAccessError:
            return "SocketAccessError";
        case QAbstractSocket::SocketResourceError:
            return "SocketResourceError";
        case QAbstractSocket::SocketTimeoutError:
            return "SocketTimeoutError";
        case QAbstractSocket::DatagramTooLargeError:
            return "DatagramTooLargeError";
        case QAbstractSocket::NetworkError:
            return "NetworkError";
        case QAbstractSocket::AddressInUseError:
            return "AddressInUseError";
        case QAbstractSocket::SocketAddressNotAvailableError:
            return "SocketAddressNotAvailableError";
        case QAbstractSocket::UnsupportedSocketOperationError:
            return "UnsupportedSocketOperationError";
        case QAbstractSocket::ProxyAuthenticationRequiredError:
            return "ProxyAuthenticationRequiredError";
        case QAbstractSocket::SslHandshakeFailedError:
            return "SslHandshakeFailedError";
        case QAbstractSocket::UnfinishedSocketOperationError:
            return "UnfinishedSocketOperationError";
        case QAbstractSocket::ProxyConnectionRefusedError:
            return "ProxyConnectionRefusedError";
        case QAbstractSocket::ProxyConnectionClosedError:
            return "ProxyConnectionClosedError";
        case QAbstractSocket::ProxyConnectionTimeoutError:
            return "ProxyConnectionTimeoutError";
        case QAbstractSocket::ProxyNotFoundError:
            return "ProxyNotFoundError";
        case QAbstractSocket::ProxyProtocolError:
            return "ProxyProtocolError";
        case QAbstractSocket::OperationError:
            return "OperationError";
        case QAbstractSocket::SslInternalError:
            return "SslInternalError";
        case QAbstractSocket::SslInvalidUserDataError:
            return "SslInvalidUserDataError";
        case QAbstractSocket::TemporaryError:
            return "TemporaryError";
        case QAbstractSocket::UnknownSocketError:
        default:
            return "UnknownSocketError";
    }
}