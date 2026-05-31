#include "Window.h"

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"

#include <assert.h>
#include <cstdint>
#include <iostream>

#undef CreateWindow

namespace raftel {

WindowingSystem WindowingSystem::s_instance = WindowingSystem();

WindowingSystem& WindowingSystem::GetInstance()
{
    return s_instance;
}

WindowingSystem::WindowingSystem()
{
    this->InitGLFW();
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

void WindowingSystem::InitGLFW() const
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

void WindowingSystem::GlobalWindowResizeCallback(GLFWwindow* window_handle, int new_width, int new_height)
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

WindowHandle WindowingSystem::RegisterWindow(GLFWwindow* window_handle, Resolution initial_resolution)
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

void WindowingSystem::RemoveWindow(size_t index)
{
    if (this->m_window_is_open[index] == false)
        return;

    glfwDestroyWindow(this->m_windows[index]);
    this->m_windows[index] = nullptr;
    this->m_window_resolutions[index] = Resolution { 0, 0 };
    this->m_window_is_open[index] = false;
    this->m_handle_generations[index]++;
}

void WindowingSystem::Update()
{
    glfwPollEvents();
    for (size_t i = 0; i < this->m_windows.size(); ++i) {
        if (this->m_windows[i] == nullptr)
            continue;

        if (glfwWindowShouldClose(this->m_windows[i])) {
            this->RemoveWindow(i);
        }
    }
}

WindowHandle WindowingSystem::CreateWindow(std::string_view name, std::uint32_t width, std::uint32_t height)
{
    GLFWwindow* window_handle = glfwCreateWindow((std::int32_t)width, (std::int32_t)height, name.data(), nullptr, nullptr);
    if (window_handle == nullptr) {
        std::cout << "Failed to create requested window" << std::endl;
    }

    glfwSetFramebufferSizeCallback(window_handle, [](GLFWwindow* window, int new_width, int new_height) {
        GetInstance().GlobalWindowResizeCallback(window, new_width, new_height);
    });

    return this->RegisterWindow(window_handle, Resolution { width, height });
}

bool WindowingSystem::HasOpenWindows() const
{
    for (size_t i = 0; i < this->m_windows.size(); ++i) {
        if (this->m_window_is_open[i])
            return true;
    }
    return false;
}

std::vector<WindowHandle> WindowingSystem::GetActiveWindowList()
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

Resolution WindowingSystem::GetWindowResolution(WindowHandle handle) const
{
    assert(handle.handle < this->m_windows.size() && handle.generation == this->m_handle_generations[handle.handle]);
    return this->m_window_resolutions[handle.handle];
}

bool WindowingSystem::IsWindowOpen(WindowHandle window_handle) const
{
    // assert(window_handle.generation == this->m_handle_generations[window_handle.handle]);
    return !glfwWindowShouldClose(this->m_windows[window_handle.handle]);
}

bool WindowingSystem::IsWindowFocused(WindowHandle window_handle) const
{
    return glfwGetWindowAttrib(this->m_windows[window_handle.handle], GLFW_FOCUSED) == GLFW_TRUE;
}

void WindowingSystem::RegisterWindowResizeCallback(WindowHandle handle, std::function<void(std::uint32_t, std::uint32_t)> callback)
{
    this->m_resize_callbacks[handle.handle].emplace_back(std::move(callback));
}

WindowHandleNativeType WindowingSystem::GetNativeWindowHandle(WindowHandle handle) const
{
    GLFWwindow* window_handle = this->m_windows[handle.handle];
#ifdef _WIN32
    return (FWD_HWND)glfwGetWin32Window(window_handle);
#elifdef __APPLE__
    return glfwGetCocoaWindow(window_handle);
#elifdef __linux__
    if (glfwGetWaylandDisplay()) {
        return glfwGetWaylandWindow(window_handle);
    } else {
        return (void*)glfwGetX11Window(window_handle);
    }
#endif
}

}
