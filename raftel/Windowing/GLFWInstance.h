#pragma once

#include "GLFW.h"

namespace raftel {

struct GLFWInstance {
private:
    static GLFWInstance instance;

    GLFWInstance();
    ~GLFWInstance();

public:
    static GLFWInstance& Instance();
};

}
