#include "filesystem.h"

#include <core/assert.h>
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

path_type filesystem::get_path_type(std::string_view path)
{
    ASSERT(path_exists(path));

    std::filesystem::file_status file_status = std::filesystem::status(path);
    switch (file_status.type()) {
    case (std::filesystem::file_type::regular):
        return path_type::file;
    case (std::filesystem::file_type::symlink):
        return path_type::link;
    case (std::filesystem::file_type::directory):
        return path_type::directory;
    case (std::filesystem::file_type::fifo):
        return path_type::FIFO;
    case (std::filesystem::file_type::socket):
        return path_type::socket;
    case (std::filesystem::file_type::block):
        return path_type::block_device;
    case (std::filesystem::file_type::character):
        return path_type::character_device;
    default:
        return path_type::unknown;
    }
}

}
