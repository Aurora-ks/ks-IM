#ifndef NET_H
#define NET_H

#include <QNetworkAccessManager>
#include <QWebSocket>
#include <functional>

enum class NetType {
    HTTP,
    WS
};

enum class HttpMethod {
    GET,
    POST,
    PUT,
    DELETE,
};

class Net {
public:
    // HTTP 回调类型
    using HttpReadByteCallback = std::function<void(int statusCode, const QByteArray &response)>;
    using HttpErrorCallback = std::function<void(int statusCode, const QString &error)>;
    // WebSocket 回调类型
    using WsTextCallback = std::function<void(const QString &text)>;
    using WsBinaryCallback = std::function<void(const QByteArray &data)>;
    using WsConectedCallback = std::function<void()>;
    using WsDisconnectedCallback = std::function<void()>;
    using WsErrorCallback = std::function<void(QAbstractSocket::SocketError error)>;
    using WsPongCallback = std::function<void(quint64 elapsedTime, const QByteArray &payload)>;

    explicit Net(NetType type, const QUrl &url = QUrl(), bool sendJson = true);

    ~Net();

    bool isSendJson() const { return sendJson_; }
    void setSendJson(bool sendJson) { sendJson_ = sendJson; }
    NetType netType() const { return type_; }

    void setUrl(const QUrl &url) { *url_ = url; }
    QUrl url() const { return *url_; }

    // http
    void setHttpReadByteCallback(const HttpReadByteCallback &callback) { httpReadByteCallback_ = callback; }
    void setHttpErrorCallback(const HttpErrorCallback &callback) { httpErrorCallback_ = callback; }

    void sendHttp(
        HttpMethod method,
        const QUrl &url,
        const QMap<QString, QString> &headers = QMap<QString, QString>(),
        const QMap<QString, QString> &query = QMap<QString, QString>(),
        const QByteArray &body = QByteArray(),
        int timeout = 3000
    );

    void get(const QMap<QString, QString> &query = QMap<QString, QString>(), const QByteArray &body = QByteArray());

    void getToUrl(const QUrl &url, const QMap<QString, QString> &query = QMap<QString, QString>(),
                  const QByteArray &body = QByteArray());

    void post(const QByteArray &body = QByteArray());

    void postToUrl(const QUrl &url, const QByteArray &body = QByteArray());

    // websocket
    void setWsConectedCallback(const WsConectedCallback &callback) { wsConnectedCallback_ = callback; }
    void setWsDisconnectedCallback(const WsDisconnectedCallback &callback) { wsDisconnectedCallback_ = callback; }
    void setWsBinaryCallback(const WsBinaryCallback &callback) { wsBinaryCallback_ = callback; }
    void setWsTextCallback(const WsTextCallback &callback) { wsTextCallback_ = callback; }
    void setWsErrorCallback(const WsErrorCallback &callback) { wsErrorCallback_ = callback; }
    void setWsPongCallback(const WsPongCallback &callback) { wsPongCallback_ = callback; }

    bool connect();

    bool connectToUrl(const QUrl &url);

    void disconnect();

    void sendWsBinary(const QByteArray &data);

    void sendWsText(const QString &message);

private:
    QNetworkAccessManager *http_{nullptr};
    HttpReadByteCallback httpReadByteCallback_;
    HttpErrorCallback httpErrorCallback_{nullptr};
    QWebSocket *ws_{nullptr};
    WsTextCallback wsTextCallback_;
    WsBinaryCallback wsBinaryCallback_;
    WsErrorCallback wsErrorCallback_;
    WsPongCallback wsPongCallback_;
    WsConectedCallback wsConnectedCallback_;
    WsDisconnectedCallback wsDisconnectedCallback_;
    QUrl *url_{nullptr};
    NetType type_;
    bool sendJson_;
};


#endif //NET_H
