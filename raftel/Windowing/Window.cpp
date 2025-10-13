#include "Window.h"
#include "GLFW/glfw3.h"

#include <assert.h>
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

WindowHandle WindowingSystem::register_window(GLFWwindow* window_handle, Resolution initial_resolution)
{
    size_t handle_index = this->m_windows.size();
    size_t generation = 0;

    this->m_windows.push_back(window_handle);
    this->m_handle_generations.push_back(0);
    this->m_window_resolutions.push_back(initial_resolution);

    return {
        .handle = handle_index,
        .generation = generation,
    };
}

void WindowingSystem::update()
{
    glfwPollEvents();
}

WindowHandle WindowingSystem::create_window(std::string_view name, size_t width, size_t height)
{
    GLFWwindow* window_handle = glfwCreateWindow(width, height, name.data(), nullptr, nullptr);
    if (window_handle == nullptr) {
        std::cout << "Failed to create requested window" << std::endl;
    }

    glfwSetFramebufferSizeCallback(window_handle, [](GLFWwindow* window, int new_width, int new_height) {
        get_instance().global_window_resize_callback(window, new_width, new_height);
    });

    return this->register_window(window_handle, Resolution { width, height });
}

bool WindowingSystem::has_open_windows() const
{
    for (size_t i = 0; i < this->m_windows.size(); ++i) {
        if (!glfwWindowShouldClose(this->m_windows[i]))
            return true;
    }
    return false;
}

bool WindowingSystem::is_window_open(WindowHandle window_handle) const
{
    assert(window_handle.generation == this->m_handle_generations[window_handle.handle]);
    return !glfwWindowShouldClose(this->m_windows[window_handle.handle]);
}

bool WindowingSystem::is_window_focused(WindowHandle window_handle) const
{
    return glfwGetWindowAttrib(this->m_windows[window_handle.handle], GLFW_FOCUSED) == GLFW_TRUE;
}

void WindowingSystem::make_window_current_context(WindowHandle window_handle) const
{
    glfwMakeContextCurrent(this->m_windows[window_handle.handle]);
}

void WindowingSystem::swap_window_framebuffers(WindowHandle window_handle) const
{
    glfwSwapBuffers(this->m_windows[window_handle.handle]);
}

}
