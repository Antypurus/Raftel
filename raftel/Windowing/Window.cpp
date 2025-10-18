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
    for (size_t i = 0; i < this->m_windows.size(); ++i) {
        if (this->m_windows[i] != window_handle)
            continue;

        this->m_window_resolutions[i] = Resolution {
            .width = (size_t)new_width,
            .height = (size_t)new_height,
        };

        for (auto& callback : this->m_resize_callbacks[i]) {
            callback(new_width, new_height);
        }
    }
}

WindowHandle WindowingSystem::register_window(GLFWwindow* window_handle, Resolution initial_resolution)
{
    size_t handle_index = this->m_windows.size();
    size_t generation = 0;

    this->m_windows.push_back(window_handle);
    this->m_handle_generations.push_back(0);
    this->m_window_is_open.push_back(true);
    this->m_window_resolutions.push_back(initial_resolution);
    this->m_resize_callbacks.push_back({});

    return {
        .handle = handle_index,
        .generation = generation,
    };
}

void WindowingSystem::remove_window(size_t index)
{
    if (this->m_window_is_open[index] == false)
        return;

    this->m_window_is_open[index] = false;
    glfwDestroyWindow(this->m_windows[index]);
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
    glfwSetWindowCloseCallback(window_handle, [](GLFWwindow* window) {
        WindowingSystem& instance = get_instance();
        for (size_t i = 0; i < instance.m_windows.size(); ++i) {
            if (instance.m_windows[i] == window) {
                instance.remove_window(i);
                break;
            }
        }
    });

    return this->register_window(window_handle, Resolution { width, height });
}

bool WindowingSystem::has_open_windows() const
{
    for (size_t i = 0; i < this->m_windows.size(); ++i) {
        if (this->m_window_is_open[i])
            return true;
    }
    return false;
}

std::vector<WindowHandle> WindowingSystem::get_active_window_list()
{
    std::vector<WindowHandle> result;
    result.reserve(this->m_windows.size());
    for (size_t i = 0; i < this->m_windows.size(); ++i) {
        if (this->m_window_is_open[i] == false)
            continue;

        result.push_back(WindowHandle {
            .handle = i,
            .generation = this->m_handle_generations[i],
        });
    }
    return result;
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

void WindowingSystem::register_window_resize_callback(WindowHandle handle, std::function<void(size_t, size_t)> callback)
{
    this->m_resize_callbacks[handle.handle].emplace_back(std::move(callback));
}

}
