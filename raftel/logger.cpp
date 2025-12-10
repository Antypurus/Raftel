#include "logger.h"

#include <iostream>
#include <print>

namespace raftel {

logger logger::s_instance = {};

logger& logger::create_logger()
{
    s_instance.m_log_file = std::ofstream("raftel_engine_log.txt", std::ios::out | std::ios::trunc);
    return logger::s_instance;
}

logger& logger::get_logger()
{
    return logger::s_instance;
}

void logger::log(std::string_view message)
{
    std::cout << message;
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

}
