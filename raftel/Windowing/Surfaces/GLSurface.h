#pragma once

#include "../GLFW.h"

namespace raftel {

class GLSurface {
private:
    GLFWwindowHandle m_window_handle = nullptr;

public:
    GLSurface(GLFWwindowHandle window_handle);

    void make_current_context() const;
    void swap_buffers() const;
    void clear() const;
};

}
