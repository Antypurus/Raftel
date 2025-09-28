#include "GLSurface.h"

#include "../GLFW.h"

#include <iostream>

namespace raftel {

GLSurface::GLSurface(GLFWwindowHandle window_handle)
    : m_window_handle(window_handle)
{
    if (GL_ARB_compute_shader) {
        std::cout << "compute shaders supported via extension" << std::endl;
    }
}

void GLSurface::make_current_context() const
{
    glfwMakeContextCurrent(m_window_handle);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }
}

void GLSurface::swap_buffers() const
{
    glfwSwapBuffers(this->m_window_handle);
}

void GLSurface::clear() const
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

}
