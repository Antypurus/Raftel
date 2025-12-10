#pragma once

#include <fstream>
#include <string_view>

namespace raftel {

struct logger {
private:
    static logger s_instance;
    std::ofstream m_log_file;

public:
    static logger& create_logger();
    static logger& get_logger();

    void log(std::string_view message);

private:
    logger() = default;
    ~logger();
};

}
