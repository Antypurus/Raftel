#pragma once

#include <string_view>

namespace raftel {

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
};

}
