#include "net.h"
#include "logger.h"
#include <QNetworkReply>
#include <QUrlQuery>
#include <QTimer>
#include <QEventLoop>

using enum NetType;
using enum HttpMethod;

Net::Net(NetType type, const QUrl &url, bool sendJson) {
    switch (type) {
        case HTTP:
            http_ = new QNetworkAccessManager();
            break;
        case WS:
            ws_ = new QWebSocket();
            QObject::connect(ws_, &QWebSocket::connected, [this] {
                if (wsConnectedCallback_) wsConnectedCallback_();
            });
            QObject::connect(ws_, QWebSocket::disconnected, [this]() {
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

void Net::sendHttp(HttpMethod method, const QUrl &url, const QMap<QString, QString> &headers,
                   const QMap<QString, QString> &query, const QByteArray &body, int timeout) {
    if (type_ != HTTP)
        LOG_FATAL("[net::send()] use on invalid net type");
    *url_ = url;
    // 设置请求参数
    QUrlQuery query_url;
    for (auto it = query.begin(); it != query.end(); ++it) {
        query_url.addQueryItem(it.key(), it.value());
    }
    url_->setQuery(query_url);
    // 设置请求头
    QNetworkRequest request(url);
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
            LOG_ERROR("[net::send()] invalid http method: {}", int(method));
    }
    // 设置超时
    QTimer *timer = new QTimer(reply);
    timer->setSingleShot(true);
    QObject::connect(timer, &QTimer::timeout, [this, reply, method]() {
        LOG_WARN("http request timeout, url: {}, method: {}", url_->toString().toStdString(), int(method));
        if (reply->isRunning()) {
            reply->abort();
            if (httpErrorCallback_) httpErrorCallback_(-1, "http request timeout");
        }
    });
    timer->start(timeout);
    // 处理响应
    QObject::connect(reply, &QNetworkReply::finished, [this, timer, reply]() {
        timer->stop();
        timer->deleteLater();

        if (reply->error() == QNetworkReply::NoError) {
            httpReadByteCallback_(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(),
                                  reply->readAll());
        } else {
            if (httpErrorCallback_)
                httpErrorCallback_(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(),
                                   reply->errorString());
        }
        reply->deleteLater();
    });
}

void Net::get(const QMap<QString, QString> &query, const QByteArray &body) {
    sendHttp(GET, *url_, QMap<QString, QString>(), query, body);
}

void Net::getToUrl(const QUrl &url, const QMap<QString, QString> &query, const QByteArray &body) {
    sendHttp(GET, url, QMap<QString, QString>(), query, body);
}

void Net::post(const QByteArray &body) {
    sendHttp(POST, *url_, QMap<QString, QString>(), QMap<QString, QString>(), body);
}

void Net::postToUrl(const QUrl &url, const QByteArray &body) {
    sendHttp(POST, url, QMap<QString, QString>(), QMap<QString, QString>(), body);
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
