#ifndef NET_H
#define NET_H

#include <QNetworkAccessManager>
#include <QWebSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <functional>

#define  RELEASE_MOD 0
#if RELEASE_MOD
#define HTTP_PREFIX "http://139.199.221.81"
#else
#define HTTP_PREFIX "http://127.0.0.1"
#endif

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

class HttpJson {
public:
    friend class HttpResponse;
    HttpJson() = default;
    ~HttpJson() = default;
    operator bool() const { return code_ == 0; }
    int code() const { return code_; }
    QString message() const { return message_; }
    QJsonDocument data() const { return data_; }
    QJsonObject dataJson() const { return dataJson_; }
    QJsonArray dataArray() const { return dataArray_; }
private:
    int code_{-1};
    QString message_;
    QJsonDocument data_;
    QJsonObject dataJson_;
    QJsonArray dataArray_;
};

class HttpResponse {
public:
    friend class Net;
    HttpResponse() = default;
    ~HttpResponse() = default;
    operator bool() const { return success_; }
    void parseJson(const QByteArray &data);
    int statusCode() const { return statusCode_; }
    QString errorString() const { return errorString_; }
    HttpJson data() const { return json_; }
private:
    bool success_;
    int statusCode_;
    QString errorString_;
    HttpJson json_;
    QByteArray dataByte_;
};


class Net {
public:
    // WebSocket 回调类型
    using WsTextCallback = std::function<void(const QString &text)>;
    using WsBinaryCallback = std::function<void(const QByteArray &data)>;
    using WsConectedCallback = std::function<void()>;
    using WsDisconnectedCallback = std::function<void()>;
    using WsErrorCallback = std::function<void(QAbstractSocket::SocketError error)>;
    using WsPongCallback = std::function<void(quint64 elapsedTime, const QByteArray &payload)>;

    explicit Net(NetType type, const QUrl &url = QUrl(), bool sendJson = true, bool receiveJson = true);

    ~Net();

    bool isSendJson() const { return sendJson_; }
    void setSendJson(bool sendJson) { sendJson_ = sendJson; }
    NetType netType() const { return type_; }

    void setUrl(const QUrl &url) { *url_ = url; }
    QUrl url() const { return *url_; }

    HttpResponse sendHttp(
        HttpMethod method,
        const QUrl &url,
        const QMap<QString, QString> &headers = QMap<QString, QString>(),
        const QMap<QString, QString> &query = QMap<QString, QString>(),
        const QByteArray &body = QByteArray(),
        int timeout = 3000
    );

    HttpResponse get(const QMap<QString, QString> &query = QMap<QString, QString>(), const QByteArray &body = QByteArray());

    HttpResponse getToUrl(const QUrl &url, const QMap<QString, QString> &query = QMap<QString, QString>(),
                  const QByteArray &body = QByteArray());

    HttpResponse post(const QByteArray &body = QByteArray());

    HttpResponse postToUrl(const QUrl &url, const QByteArray &body = QByteArray());

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
    bool receiveJson_;
};


#endif //NET_H
