#include <QApplication>
#include <Ela/ElaApplication.h>
#include <Ela/ElaTheme.h>
#include "LoginWindow.h"
#include "MainWindow.h"
#include "logger.h"
#include "user.h"
#include "setting.h"
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
            logger->debug("[QT] {} ({}:{})", message, file, context.line);
            break;
        case QtInfoMsg:
            logger->info("[QT] {}", message);
            break;
        case QtWarningMsg:
            logger->warn("[QT] {} ({}:{})", message, file, context.line);
            break;
        case QtCriticalMsg:
            logger->error("[QT] {} ({}:{})", message, file, context.line);
            break;
        case QtFatalMsg:
            logger->critical("[QT] FATAL: {} ({}:{})", message, file, context.line);
            logger->flush();
            exit(EXIT_FAILURE);
    }
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

    QObject::connect(&a, &QApplication::aboutToQuit, []() {
       LOG_INFO("application quit");
    });

    LoginWindow *w = new LoginWindow();
    MainWindow *mw = nullptr;
    QObject::connect(w, &LoginWindow::loginSuccess, [&](int64_t uid) {
        LOG_INFO("u[{}] show main window", uid);
        User::SetUid(uid);
        setting::SetUserPath(setting::GetDirPath() + QString("/%1").arg(uid));
        mw = new MainWindow(uid);
        mw->moveToCenter();
        mw->show();
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
    LOG_INFO("show login window");

    return QApplication::exec();
}
