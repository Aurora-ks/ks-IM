#include "logger.h"
#include <QDir>
#include <QDateTime>
#include <exception>
#include <QCoreApplication>

// 自定义异常处理器
void customTerminateHandler() {
    const auto logger = Logger::instance().L();
    logger->critical("!!! 未捕获异常导致程序终止 !!!");
    // 尝试获取异常信息
    try {
        if (const auto ex = std::current_exception()) {
            std::rethrow_exception(ex);
        }
    } catch (const std::exception& e) {
        logger->critical("异常类型: {}", typeid(e).name());
        logger->critical("异常信息: {}", e.what());
    } catch (...) {
        logger->critical("未知异常类型");
    }

    // 刷新日志并退出
    logger->flush();
    std::exit(EXIT_FAILURE);
}

void Logger::init(bool enableConsole, bool enableFile, spdlog::level::level_enum level) {
    try {
        // 设置全局异常处理器
        std::set_terminate(customTerminateHandler);

        // 创建线程池（128k条目缓存，2个后台线程）
        threadPool_ = std::make_shared<spdlog::details::thread_pool>(131072, 2);

        // 配置控制台输出（仅Debug模式且启用控制台）
// #ifdef QT_DEBUG
        if (enableConsole) {
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_pattern("%^[%Y-%m-%d %T.%e] [%l] [thr %t]%$ %v");
            sinks_.push_back(console_sink);
        }
// #endif

        // 配置文件输出
        if (enableFile) {
            // 日志文件：./logs/YYYY-MM-dd.log
            const QString dateStr = QDateTime::currentDateTime().toString("yyyy-MM-dd");
            const QString logDir = QCoreApplication::applicationDirPath()
                                 + "/logs";
            if (!QDir(logDir).exists()) {
                if (!QDir().mkpath(logDir)) {
                    throw std::runtime_error("无法创建日志目录: " + logDir.toStdString());
                }
            }
            // 滚动日志文件配置（每个文件50MB，保留10个）
            auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                logDir.toStdString() + "/" + dateStr.toStdString() + ".log", 1024 * 1024 * 50, 10
            );
            file_sink->set_pattern("[%Y-%m-%d %T.%e] [%l] [thr %t] %v");
            sinks_.push_back(file_sink);
        }

        // 创建异步日志器
        logger_ = std::make_shared<spdlog::async_logger>(
            "qt_app",                // 日志器名称
            begin(sinks_), end(sinks_),  // 输出目标
            threadPool_,            // 线程池
            spdlog::async_overflow_policy::block  // 队列满时阻塞
        );

        // 设置日志级别
        logger_->set_level(level);

        // 设置刷新策略（每5秒或遇到error立即刷新）
        logger_->flush_on(spdlog::level::err);
        spdlog::flush_every(std::chrono::seconds(5));

        // 注册为全局日志器
        spdlog::register_logger(logger_);
        spdlog::set_default_logger(logger_);

    } catch (const std::exception& e) {
        // 如果日志初始化失败，使用Qt原生机制告警
        qFatal("日志系统初始化失败: %s", e.what());
    }
}

void Logger::shutdown() {
    if (logger_) {
        logger_->flush();
        spdlog::drop("qt_app");  // 注销日志器
    }
    spdlog::shutdown();  // 安全关闭所有资源
}
