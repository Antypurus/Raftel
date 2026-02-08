#include "Window.h"
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"

#include <assert.h>
#include <cstdint>
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
}

WindowingSystem::~WindowingSystem()
{
    for (GLFWwindow* window : this->m_windows) {
        if (window == nullptr)
            continue;
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}

void WindowingSystem::init_glfw() const
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

void WindowingSystem::global_window_resize_callback(GLFWwindow* window_handle, int new_width, int new_height)
{
    for (size_t i = 0; i < this->m_windows.size(); ++i) {
        if (this->m_windows[i] != window_handle)
            continue;

        this->m_window_resolutions[i] = Resolution {
            .width = (std::uint32_t)new_width,
            .height = (std::uint32_t)new_height,
        };

        for (auto& callback : this->m_resize_callbacks[i]) {
            callback((std::uint32_t)new_width, (std::uint32_t)new_height);
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

    glfwDestroyWindow(this->m_windows[index]);
    this->m_windows[index] = nullptr;
    this->m_window_resolutions[index] = Resolution { 0, 0 };
    this->m_window_is_open[index] = false;
    this->m_handle_generations[index]++;
}

void WindowingSystem::update()
{
    glfwPollEvents();
    for (size_t i = 0; i < this->m_windows.size(); ++i) {
        if (this->m_windows[i] == nullptr)
            continue;

        if (glfwWindowShouldClose(this->m_windows[i])) {
            this->remove_window(i);
        }
    }
}

WindowHandle WindowingSystem::create_window(std::string_view name, std::uint32_t width, std::uint32_t height)
{
    GLFWwindow* window_handle = glfwCreateWindow((std::int32_t)width, (std::int32_t)height, name.data(), nullptr, nullptr);
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

Resolution WindowingSystem::get_window_resolution(WindowHandle handle) const
{
    assert(handle.handle < this->m_windows.size() && handle.generation == this->m_handle_generations[handle.handle]);
    return this->m_window_resolutions[handle.handle];
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

void WindowingSystem::register_window_resize_callback(WindowHandle handle, std::function<void(std::uint32_t, std::uint32_t)> callback)
{
    this->m_resize_callbacks[handle.handle].emplace_back(std::move(callback));
}

WindowHandleNativeType WindowingSystem::get_native_window_handle(WindowHandle handle) const
{
    GLFWwindow* window_handle = this->m_windows[handle.handle];
#ifdef _WIN32
    return glfwGetWin32Window(window_handle);
#elifdef __APPLE__
    return glfwGetCocoaWindow(window_handle);
#elifdef __linux__
    if (glfwGetWaylandDisplay()) {
        return glfwGetWaylandWindow(window_handle);
    } else {
        return (void*)glfwGetX11Window(window_handle);
    }
#endif
    return nullptr;
}

}
