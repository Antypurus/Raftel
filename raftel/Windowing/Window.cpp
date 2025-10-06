#include "Window.h"
#include "GLFW/glfw3.h"

#include <iostream>

namespace raftel {

WindowingSystem WindowingSystem::s_instance = WindowingSystem();

WindowingSystem& WindowingSystem::get_instance()
{
    return s_instance;
}

WindowingSystem::WindowingSystem()
{
    this->init_glfw();
    this->load_opengl();
}

WindowingSystem::~WindowingSystem()
{
    for (GLFWwindow* window : this->m_windows) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}

void WindowingSystem::init_glfw() const
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
}

void WindowingSystem::load_opengl() const
{
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* dummy = glfwCreateWindow(1, 1, "", nullptr, nullptr);
    if (dummy == nullptr) {
        std::cout << "Error, Faild to create Dummy Window for OpenGL Function Loading" << std::endl;
    }
    glfwMakeContextCurrent(dummy);

    // NOTE(Tiago): an OpenGL context must be set for us to be able to load OpenGL from it
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to load OpenGL" << std::endl;
    } else {
        std::cout << "OpenGL has been loaded" << std::endl;
    }

    glfwDestroyWindow(dummy);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
}

void WindowingSystem::global_window_resize_callback(GLFWwindow* window_handle, int new_width, int new_height)
{
    for (GLFWwindow* window : m_windows) {
        if (window != window_handle)
            continue;

        std::cout << "Window resized to " << new_width << "x" << new_height << std::endl;
    }
}

void WindowingSystem::update()
{
    glfwPollEvents();
}

size_t WindowingSystem::create_window(std::string_view name, int width, int height)
{
    GLFWwindow* window_handle = glfwCreateWindow(width, height, name.data(), nullptr, nullptr);
    if (window_handle == nullptr) {
        std::cout << "Failed to create requested window" << std::endl;
    }

    glfwSetFramebufferSizeCallback(window_handle, [](GLFWwindow* window, int new_width, int new_height) {
        get_instance().global_window_resize_callback(window, new_width, new_height);
    });

    this->m_windows.push_back(window_handle);
    return this->m_windows.size() - 1;
}

bool WindowingSystem::has_open_windows() const
{
    for (size_t i = 0; i < this->m_windows.size(); ++i) {
        if (this->is_window_open(i))
            return true;
    }
    return false;
}

bool WindowingSystem::is_window_open(int window_handle) const
{
    return !glfwWindowShouldClose(this->m_windows[window_handle]);
}

bool WindowingSystem::is_window_focused(int window_handle) const
{
    return glfwGetWindowAttrib(this->m_windows[window_handle], GLFW_FOCUSED) == GLFW_TRUE;
}

void WindowingSystem::make_window_current_context(int window_handle) const
{
    glfwMakeContextCurrent(this->m_windows[window_handle]);
}

void WindowingSystem::swap_window_framebuffers(int window_handle) const
{
    glfwSwapBuffers(this->m_windows[window_handle]);
}

}
