#include <QApplication>
#include <Ela/ElaApplication.h>
#include <Ela/ElaTheme.h>
#include <Ela/ElaMessageBar.h>
#include "LoginWindow.h"
#include "MainWindow.h"
#include "logger.h"
#include "user.h"
#include "setting.h"
#include "ws.h"
#define DEBUG 1
// Qt消息处理器（将Qt日志转发到spdlog）
void qtMessageHandler(QtMsgType type,
                      const QMessageLogContext &context,
                      const QString &msg) {
    const auto logger = ::Logger::instance().L();
    const std::string message = msg.toStdString();
    const char *file = context.file ? context.file : "";

    switch (type) {
        case QtDebugMsg:
            logger->debug("[QT] [uid:{}] {} ({}:{})", User::GetUid(), message, file, context.line);
            break;
        case QtInfoMsg:
            logger->info("[QT] [uid:{}] {}", User::GetUid(), message);
            break;
        case QtWarningMsg:
            logger->warn("[QT] [uid:{}] {} ({}:{})", User::GetUid(), message, file, context.line);
            break;
        case QtCriticalMsg:
            logger->error("[QT] [uid:{}] {} ({}:{})", User::GetUid(), message, file, context.line);
            break;
        case QtFatalMsg:
            logger->critical("[QT] FATAL [uid:{}] : {} ({}:{})", User::GetUid(), message, file, context.line);
            logger->flush();
            exit(EXIT_FAILURE);
    }
}
// 连接WebSocket
bool wsConnect(){
//    auto resp = Net::GetTo("/message_service");
//    if(!resp){
//        LOG_ERROR("[Net] ws connect failed, err:{}", resp.errorString().toStdString());
//        return false;
//    }
//    auto data = resp.data();
//    if(!data){
//        LOG_ERROR("[Net] ws connect failed, msg:{}", data.message().toStdString());
//        return false;
//    }
//    auto json = data.dataJson();
//    if(json.empty()) qDebug() << "Empty";
//    QString url = QString("ws://%1:%2/login/%3").arg(json["ip"].toString()).arg(json["port"].toInt()).arg(User::GetUid());
    QString url = QString("ws://127.0.0.1:9999/login/%3").arg(User::GetUid());
    return WsIns.connectToServer(url);
}

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    eApp->init();

    Logger::instance().init(
#if DEBUG
        true
#else
        false
#endif
        , true,
#if DEBUG
        spdlog::level::debug
#else
        spdlog::level::info
#endif
    );
    qInstallMessageHandler(qtMessageHandler);

    LoginWindow *w = new LoginWindow();
    MainWindow *mw = nullptr;
    QObject::connect(w, &LoginWindow::loginSuccess, [&](int64_t uid) {
        User::SetUid(uid);
        setting::SetUserPath(setting::GetDirPath() + QString("/%1").arg(uid));
        mw = new MainWindow(uid);
        mw->moveToCenter();
        mw->show();

        bool connected = wsConnect();
        if(!connected)
            ElaMessageBar::error(ElaMessageBarType::Top, "网络错误", "连接服务器失败", 2000, mw);

        setting *setting = setting::getDBInstance(setting::GetUserPath() + "/setting.db");
        auto [val, err] = setting->valueDB("ThemeMode", "0");
        if(err) qCritical() << "[setting] load ThemeMode err:" << err.text() << " type:" << err.type();
        if(val == "1")
            eTheme->setThemeMode(ElaThemeType::ThemeMode::Dark);
        else
            eTheme->setThemeMode(ElaThemeType::ThemeMode::Light);

        w->close();
        w->deleteLater();
    });
    QObject::connect(w, &LoginWindow::destroyed, [&]() {
        if(mw && !mw->isVisible()) a.quit();
    });

    w->moveToCenter();
    w->show();

    return QApplication::exec();
}
