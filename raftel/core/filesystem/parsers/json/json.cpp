#include "json.h"

#include <core/logger.h>

#include <fstream>

namespace raftel::parsers {

void parse_json(std::string_view path)
{
    LOG_ERROR("test");

    std::ifstream file(path.data());
}

}
