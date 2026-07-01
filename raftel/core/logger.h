#pragma once

#include <format>
#include <fstream>
#include <string_view>

namespace raftel {

struct Logger {
public:
    enum class LogLevel {
        Success,
        Error,
        Warning,
        Info
    };

private:
    static Logger s_Instance;
    std::ofstream m_LogFile;

public:
    static Logger& CreateLogger();
    static Logger& GetLogger();

    void Log(LogLevel Level, std::string_view Message);

    template <typename... ArgT>
    void Log(LogLevel Level, std::string_view Message, ArgT&&... Params);

private:
    Logger()
        = default;
    ~Logger();
};

template <typename... ArgT>
void Logger::Log(LogLevel p_Level, std::string_view p_Message, ArgT&&... p_Params)
{
    std::string logMessage = std::vformat(p_Message, std::make_format_args(p_Params...));
    this->Log(p_Level, logMessage);
}

const char* ExtractFilename(const char* filepath);
std::string GetTimeString();

}

#ifdef _WIN32
    #define LOG(p_Level, p_Message, p_LevelStr, ...)                                                                   \
        {                                                                                                              \
            auto& logger_ = Logger::GetLogger();                                                                       \
            auto time_ = GetTimeString();                                                                              \
            auto file_ = raftel::ExtractFilename(__FILE__);                                                            \
            logger_.Log(p_Level, "[{}][{}]{}@{} -> " p_Message "\n", time_, p_LevelStr, file_, __LINE__, __VA_ARGS__); \
        }
#else
    #define LOG(p_Level, p_Message, p_LevelStr, ...)                                                                                 \
        {                                                                                                                            \
            auto& logger_ = Logger::GetLogger();                                                                                     \
            auto time_ = GetTimeString();                                                                                            \
            auto file_ = raftel::ExtractFilename(__FILE__);                                                                          \
            logger_.Log(p_Level, "[{}][{}]{}@{} -> " p_Message "\n", time_, p_LevelStr, file_, __LINE__ __VA_OPT__(, ) __VA_ARGS__); \
        }
#endif

#define LOGGING_ENABLE 1
#if LOGGING_ENABLE
    #define LOG_SUCCESS(Message, ...) LOG(Logger::LogLevel::Success, Message, "SUCCESS", __VA_ARGS__)
    #define LOG_WARNING(Message, ...) LOG(Logger::LogLevel::Warning, Message, "WARNING", __VA_ARGS__)
    #define LOG_ERROR(Message, ...) LOG(Logger::LogLevel::Error, Message, "ERROR", __VA_ARGS__)
    #define LOG_INFO(Message, ...) LOG(Logger::LogLevel::Info, Message, "INFO", __VA_ARGS__)
    #define LOG_DEBUG(Message, ...) LOG(Logger::LogLevel::Info, Message, "INFO", __VA_ARGS__)
#else
    #define LOG_SUCCESS(Message, ...)
    #define LOG_WARNING(Message, ...)
    #define LOG_ERROR(Message, ...)
    #define LOG_INFO(Message, ...)
    #define LOG_DEBUG(Message, ...)
#endif
