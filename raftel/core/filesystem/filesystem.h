#pragma once

#include <string>
#include <string_view>
#include <vector>

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
    static bool PathExists(std::string_view path);

    static path_type GetPathType(std::string_view path);

    static std::string_view GetParentDir(std::string_view path);

    static bool CreatePath(std::string_view path, bool createMissing = true);

    static std::vector<std::uint8_t> ReadFile(std::string_view filepath);

    static std::string GetCWD();
};

}
