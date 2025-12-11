#pragma once

#include <format>
#include <fstream>
#include <string_view>

namespace raftel {

struct logger {
public:
    enum class log_level {
        success,
        error,
        warning,
        info
    };

private:
    static logger s_instance;
    std::ofstream m_log_file;

public:
    static logger& create_logger();
    static logger& get_logger();

    void log(log_level level, std::string_view message);

    template <typename... ArgT>
    void log(log_level level, std::string_view message, ArgT&&... params);

private:
    logger()
        = default;
    ~logger();
};

template <typename... ArgT>
void logger::log(log_level level, std::string_view message, ArgT&&... params)
{
    std::string log_message = std::vformat(message, std::make_format_args(params...));
    this->log(level, log_message);
}

const char* extract_filename(const char* filepath);

}

#define LOG(level, Message, level_str, ...)                                                                \
    {                                                                                                      \
        auto& logger = logger::get_logger();                                                               \
        auto time = std::time(nullptr);                                                                    \
        auto file = raftel::extract_filename(__FILE__);                                                    \
        logger.log(level, "[{}][{}]{}@{} -> " Message "\n", time, level_str, file, __LINE__, __VA_ARGS__); \
    }

#define LOGGING_ENABLE 1
#if LOGGING_ENABLE
    #define LOG_SUCCESS(Message, ...) LOG(logger::log_level::success, Message, "SUCCESS", __VA_ARGS__)
    #define LOG_WARNING(Message, ...) LOG(logger::log_level::warning, Message, "WARNING", __VA_ARGS__)
    #define LOG_ERROR(Message, ...) LOG(logger::log_level::error, Message, "ERROR", __VA_ARGS__)
    #define LOG_INFO(Message, ...) LOG(logger::log_level::info, Message, "INFO", __VA_ARGS__)
#else
    #define LOG_SUCCESS(Message, ...)
    #define LOG_WARNING(Message, ...)
    #define LOG_ERROR(Message, ...)
    #define LOG_INFO(Message, ...)
#endif
