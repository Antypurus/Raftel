#pragma once

#include <string_view>

namespace raftel {

enum class path_type {
    file,
    link,
    directory,
    FIFO,
    socket,
    block_device,
    character_device,
    unknown
};

class filesystem {
public:
    /**
     * @brief Check if a given path exists, if current
     * user doesnt not have permission to access path
     * false will also be returned.
     *
     * @param path path to check
     * @return true if path exists, false if the path does
     * not exist of the user does not have permissions to access
     * it
     */
    static bool path_exists(std::string_view path);

    static path_type get_path_type(std::string_view path);

    static std::string_view get_parent_dir(std::string_view path);

    static bool create_directory(std::string_view path);
};

}
