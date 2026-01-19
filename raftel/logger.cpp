#include "logger.h"

#include <iostream>
#include <string.h>

namespace raftel {

#define ANSI_RESET "\033[0m"
#define ANSI_SUCCESS_FG "\033[92m"
#define ANSI_WARNING_FG "\033[93m"
#define ANSI_ERROR_FG "\033[91m"
#define ANSI_INFO_FG "\033[35m"

logger logger::s_instance = {};

logger& logger::create_logger()
{
    if (!s_instance.m_log_file.is_open()) {
        s_instance.m_log_file = std::ofstream("raftel_engine_log.txt", std::ios::out | std::ios::trunc);
    }
    return logger::s_instance;
}

logger& logger::get_logger()
{
    return logger::create_logger();
}

void logger::log(log_level level, std::string_view message)
{
    switch (level) {
    case (log_level::success): {
        std::cout << ANSI_SUCCESS_FG;
        break;
    }
    case (log_level::warning): {
        std::cout << ANSI_WARNING_FG;
        break;
    }
    case (log_level::error): {
        std::cout << ANSI_ERROR_FG;
        break;
    }
    case (log_level::info): {
        std::cout << ANSI_INFO_FG;
        break;
    }
    default:
        break;
    }

    std::cout << message;
    std::cout << ANSI_RESET;
    if (this->m_log_file.is_open()) {
        this->m_log_file << message;
    }
}

logger::~logger()
{
    if (this->m_log_file.is_open()) {
        this->m_log_file.close();
    }
}

const char* extract_filename(const char* filepath)
{
    auto size = strlen(filepath);
    for (size_t i = size; i + 1 >= 0; --i) {
        if (filepath[i] == '/') {
            return filepath + i + 1;
        }
    }
    return nullptr;
}

}
