#ifndef LOGGER_H
#define LOGGER_H

#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <QString>

#define LOG_TRACE(format, ...) logger::instance().L()->trace(format, ##__VA_ARGS__)
#define LOG_DEBUG(format, ...) logger::instance().L()->debug(format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) logger::instance().L()->info(format, ##__VA_ARGS__)
#define LOG_WARN(format, ...) logger::instance().L()->warn(format " [{}:{}]", ##__VA_ARGS__, __FILE__, __LINE__)
#define LOG_ERROR(format, ...) logger::instance().L()->error(format " [{}:{}]", ##__VA_ARGS__, __FILE__, __LINE__)
#define LOG_CRITICAL(format, ...) logger::instance().L()->critical(format " [{}:{}]", ##__VA_ARGS__, __FILE__, __LINE__)
#define LOG_FATAL(format, ...) \
    do{ \
        logger::instance().L()->critical("[FATAL] " format " [{}:{}]", ##__VA_ARGS__, __FILE__, __LINE__); \
        logger::instance().L()->flush(); \
        std::exit(EXIT_FAILURE); \
    }while(0)

class logger {
public:
    // 获取单例实例
    static logger &instance() {
        static logger instance;
        return instance;
    }

    // 初始化日志系统
    void init(bool enableConsole = true,
              bool enableFile = true,
              spdlog::level::level_enum level = spdlog::level::debug);

    // 关闭日志系统
    void shutdown();

    // 获取日志器（供业务代码使用）
    std::shared_ptr<spdlog::logger> L() { return logger_; }

    logger(const logger &) = delete;

    logger &operator=(const logger &) = delete;

private:
    logger() = default;

    ~logger() { shutdown(); }

    std::shared_ptr<spdlog::logger> logger_; // 主日志器实例
    std::vector<spdlog::sink_ptr> sinks_; // 日志输出目标集合
    std::shared_ptr<spdlog::details::thread_pool> threadPool_; // 异步线程池
};

#endif //LOGGER_H
