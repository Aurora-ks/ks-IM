#include <QApplication>
#include <Ela/ElaApplication.h>
#include "LoginWindow.h"
#include "MainWindow.h"
#include "logger.h"
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
    MainWindow mw;
    QObject::connect(w, &LoginWindow::loginSuccess, [&](QString uid) {
        LOG_INFO("[u{}] show main window", uid.toStdString());
        mw.moveToCenter();
        mw.show();
        w->close();
        w->deleteLater();
    });
    QObject::connect(w, &LoginWindow::destroyed, [&]() {
        if(!mw.isVisible()) a.quit();
    });

    w->moveToCenter();
    w->show();
    LOG_INFO("show login window");

    return QApplication::exec();
}
