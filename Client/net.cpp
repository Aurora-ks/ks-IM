#include "net.h"
#include "logger.h"
#include <QNetworkReply>
#include <QUrlQuery>
#include <QTimer>
#include <QEventLoop>

using enum NetType;
using enum HttpMethod;

void HttpResponse::parseJson(const QByteArray &data) {
    if (data.isEmpty()) return;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
    if(jsonDoc.isNull()) return;
    json_.data_ = jsonDoc;
    QJsonObject jsonObject = jsonDoc.object();
    json_.code_ = jsonObject["code"].toInt();
    json_.message_ = jsonObject["message"].toString();
    if(jsonObject["data"].isNull()) return;
    QJsonDocument dataDoc = QJsonDocument::fromJson(QByteArray::fromBase64(jsonObject["data"].toString().toUtf8()));
    if(dataDoc.isNull()) return;
    if(dataDoc.isObject()) json_.dataJson_ = dataDoc.object();
    else if(dataDoc.isArray()) json_.dataArray_ = dataDoc.array();
}

Net::Net(NetType type, const QUrl &url, bool sendJson, bool receiveJson) {
    switch (type) {
        case HTTP:
            http_ = new QNetworkAccessManager();
            break;
        case WS:
            ws_ = new QWebSocket();
            QObject::connect(ws_, &QWebSocket::connected, [this] {
                if (wsConnectedCallback_) wsConnectedCallback_();
            });
            QObject::connect(ws_, &QWebSocket::disconnected, [this]() {
                if (wsDisconnectedCallback_) wsDisconnectedCallback_();
            });
            QObject::connect(ws_, &QWebSocket::binaryMessageReceived, [this](const QByteArray &data) {
                if (wsBinaryCallback_) wsBinaryCallback_(data);
            });
            QObject::connect(ws_, &QWebSocket::textMessageReceived, [this](const QString &text) {
                if (wsTextCallback_) wsTextCallback_(text);
            });
            QObject::connect(ws_, &QWebSocket::errorOccurred, [this](QAbstractSocket::SocketError error) {
                if (wsErrorCallback_) wsErrorCallback_(error);
            });
            QObject::connect(ws_, &QWebSocket::pong, [this](quint64 elapsedTime, const QByteArray &payload) {
                if (wsPongCallback_) wsPongCallback_(elapsedTime, payload);
            });
            break;
        default:
            LOG_FATAL("Invalid net type");
    }
    url_ = new QUrl(url);
    type_ = type;
    sendJson_ = sendJson;
    receiveJson_ = receiveJson;
}

Net::~Net() {
    delete url_;
    switch (type_) {
        case HTTP:
            delete http_;
            break;
        case WS:
            ws_->close();
            delete ws_;
            break;
        default:
            LOG_FATAL("[net] instance has invalid net type");
    }
}

HttpResponse Net::sendHttp(HttpMethod method, const QUrl &url, const QMap<QString, QString> &headers,
                           const QMap<QString, QString> &query, const QByteArray &body, int timeout) {
    if (type_ != HTTP)
        LOG_FATAL("[net::send()] use on invalid net type");
    *url_ = url;
    HttpResponse result;
    // 设置请求参数
    QUrlQuery query_url;
    for (auto it = query.begin(); it != query.end(); ++it) {
        query_url.addQueryItem(it.key(), it.value());
    }
    url_->setQuery(query_url);
    // 设置请求头
    QNetworkRequest request(*url_);
    for (auto it = headers.begin(); it != headers.end(); ++it) {
        request.setRawHeader(it.key().toUtf8(), it.value().toUtf8());
    }
    if (sendJson_)
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    // 发送请求
    QNetworkReply *reply = nullptr;
    switch (method) {
        case GET:
            reply = http_->get(request, body);
            break;
        case POST:
            reply = http_->post(request, body);
            break;
        case PUT:
            reply = http_->put(request, body);
            break;
        case DELETE:
            reply = http_->deleteResource(request);
            break;
        default:
            result.statusCode_ = 0;
            result.success_ = false;
            result.errorString_ = "Unsupported HTTP method";
            return result;
    }
    // 设置超时
    QEventLoop loop;
    QTimer *timer = new QTimer(reply);
    timer->setSingleShot(true);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(timer, &QTimer::timeout, [this, reply, method, &result, &loop]() {
        LOG_WARN("http request timeout, url: {}, method: {}", url_->toString().toStdString(), int(method));
        if (reply->isRunning()) {
            reply->abort();
            result.success_ = false;
            result.errorString_ = "Request timeout";
            loop.quit();
        }
    });
    // 启动定时器和时间循环
    timer->start(timeout);
    loop.exec(QEventLoop::ExcludeUserInputEvents);
    // 处理响应
    if (reply) {
        // 非超时
        result.statusCode_ = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (reply->error() == QNetworkReply::NoError) {
            result.success_ = true;
            if (receiveJson_) result.parseJson(reply->readAll());
            else result.dataByte_ = reply->readAll();
        } else {
            result.success_ = false;
            result.errorString_ = reply->errorString();
        }
    }
    reply->deleteLater();
    return result;
}

HttpResponse Net::get(const QMap<QString, QString> &query, const QByteArray &body) {
    return sendHttp(GET, *url_, QMap<QString, QString>(), query, body);
}

HttpResponse Net::getToUrl(const QUrl &url, const QMap<QString, QString> &query, const QByteArray &body) {
    return sendHttp(GET, url, QMap<QString, QString>(), query, body);
}

HttpResponse Net::post(const QByteArray &body) {
    return sendHttp(POST, *url_, QMap<QString, QString>(), QMap<QString, QString>(), body);
}

HttpResponse Net::postToUrl(const QUrl &url, const QByteArray &body) {
    return sendHttp(POST, url, QMap<QString, QString>(), QMap<QString, QString>(), body);
}

bool Net::connect() {
    if (type_ != WS)
        LOG_FATAL("[net::connect()] use on invalid net type");
    // 阻塞等待连接完成
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    QObject::connect(ws_, &QWebSocket::connected, &loop, &QEventLoop::quit);
    QObject::connect(ws_, &QWebSocket::errorOccurred, &loop, &QEventLoop::quit);

    timer.start(3000);
    ws_->open(*url_);
    loop.exec();

    timer.stop();
    return ws_->state() == QAbstractSocket::ConnectedState;
}

bool Net::connectToUrl(const QUrl &url) {
    *url_ = url;
    return connect();
}

void Net::disconnect() {
    if (type_ != WS)
        LOG_FATAL("[net::disconnect()] use on invalid net type");
    ws_->close();
}

void Net::sendWsBinary(const QByteArray &data) {
    if (type_ != WS)
        LOG_FATAL("[net::sendWsBinary()] use on invalid net type");
    ws_->sendBinaryMessage(data);
}

void Net::sendWsText(const QString &message) {
    if (type_ != WS)
        LOG_FATAL("[net::sendWsText()] use on invalid net type");
    ws_->sendTextMessage(message);
}
