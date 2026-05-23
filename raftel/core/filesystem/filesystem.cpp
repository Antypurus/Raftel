#include "filesystem.h"

#include <core/logger.h>

#include <filesystem>

namespace raftel {

bool filesystem::path_exists(std::string_view path)
{
    // asside from a file existing or not exists can
    // error due to lack of permissions. The default
    // method would throw, but the erro code overload
    // puts that error into std::error_code type variable
    // so in case that variable holds a value we return
    // false as if the file doesnt exist.
    std::error_code error;
    bool result = std::filesystem::exists(path, error);
    if (error) {
        LOG_DEBUG("{}", error.message());
        return false;
    }
    return result;
}

}
