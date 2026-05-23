#include "json.h"

#include <fstream>
#include <logger.h>

namespace raftel::parsers {

void parse_json(std::string_view path)
{
    LOG_ERROR("test");

    std::ifstream file(path.data());
}

}
