#include "logger.h"

#include <chrono>
#include <ctime>
#include <iostream>
#include <string.h>

namespace raftel {

#define ANSI_RESET "\033[0m"
#define ANSI_SUCCESS_FG "\033[92m"
#define ANSI_WARNING_FG "\033[93m"
#define ANSI_ERROR_FG "\033[91m"
#define ANSI_INFO_FG "\033[35m"

Logger Logger::s_Instance = { };

Logger& Logger::CreateLogger()
{
    if (!s_Instance.m_LogFile.is_open()) {
        s_Instance.m_LogFile = std::ofstream("raftel_engine_log.txt", std::ios::out | std::ios::trunc);
    }
    return Logger::s_Instance;
}

Logger& Logger::GetLogger()
{
    return Logger::CreateLogger();
}

void Logger::Log(LogLevel p_Level, std::string_view p_Message)
{
    switch (p_Level) {
    case (LogLevel::Success): {
        std::cout << ANSI_SUCCESS_FG;
        break;
    }
    case (LogLevel::Warning): {
        std::cout << ANSI_WARNING_FG;
        break;
    }
    case (LogLevel::Error): {
        std::cout << ANSI_ERROR_FG;
        break;
    }
    case (LogLevel::Info): {
        std::cout << ANSI_INFO_FG;
        break;
    }
    default:
        break;
    }

    std::cout << p_Message;
    std::cout << ANSI_RESET;
    if (this->m_LogFile.is_open()) {
        this->m_LogFile << p_Message;
    }
}

Logger::~Logger()
{
    if (this->m_LogFile.is_open()) {
        this->m_LogFile.close();
    }
}

const char* ExtractFilename(const char* p_Filepath)
{
    auto size = strlen(p_Filepath);
    for (size_t i = size; i + 1 >= 0; --i) {
        if (p_Filepath[i] == '/') {
            return p_Filepath + i + 1;
        }
    }
    return nullptr;
}

std::string GetTimeString()
{
    std::chrono::time_point now = std::chrono::system_clock::now();
    return std::format("{:%F %T}", now);
}

}
