#include "Window.h"

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"

#include <assert.h>
#include <cstdint>
#include <iostream>

#undef CreateWindow

namespace raftel {

WindowingSystem WindowingSystem::s_Instance = WindowingSystem();

WindowingSystem& WindowingSystem::GetInstance()
{
    return s_Instance;
}

WindowingSystem::WindowingSystem()
{
    this->InitGLFW();
}

WindowingSystem::~WindowingSystem()
{
    for (GLFWwindow* window : this->m_Windows) {
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

void WindowingSystem::GlobalWindowResizeCallback(GLFWwindow* p_WindowHandle, int p_NewWidth, int p_NewHeight)
{
    for (size_t i = 0; i < this->m_Windows.size(); ++i) {
        if (this->m_Windows[i] != p_WindowHandle)
            continue;

        this->m_WindowResolutions[i] = Resolution {
            .Width = (std::uint32_t)p_NewWidth,
            .Height = (std::uint32_t)p_NewHeight,
        };

        for (auto& callback : this->m_ResizeCallbacks[i]) {
            callback((std::uint32_t)p_NewWidth, (std::uint32_t)p_NewHeight);
        }
    }
}

WindowHandle WindowingSystem::RegisterWindow(GLFWwindow* p_WindowHandle, Resolution p_InitialResolution)
{
    size_t handleIndex = this->m_Windows.size();
    size_t generation = 0;

    this->m_Windows.push_back(p_WindowHandle);
    this->m_HandleGenerations.push_back(0);
    this->m_WindowIsOpen.push_back(true);
    this->m_WindowResolutions.push_back(p_InitialResolution);
    this->m_ResizeCallbacks.push_back({});

    return {
        .Handle = handleIndex,
        .Generation = generation,
    };
}

void WindowingSystem::RemoveWindow(size_t p_Index)
{
    if (this->m_WindowIsOpen[p_Index] == false)
        return;

    glfwDestroyWindow(this->m_Windows[p_Index]);
    this->m_Windows[p_Index] = nullptr;
    this->m_WindowResolutions[p_Index] = Resolution { 0, 0 };
    this->m_WindowIsOpen[p_Index] = false;
    this->m_HandleGenerations[p_Index]++;
}

void WindowingSystem::Update()
{
    glfwPollEvents();
    for (size_t i = 0; i < this->m_Windows.size(); ++i) {
        if (this->m_Windows[i] == nullptr)
            continue;

        if (glfwWindowShouldClose(this->m_Windows[i])) {
            this->RemoveWindow(i);
        }
    }
}

WindowHandle WindowingSystem::CreateWindow(std::string_view p_Name, std::uint32_t p_Width, std::uint32_t p_Height)
{
    GLFWwindow* windowHandle = glfwCreateWindow((std::int32_t)p_Width, (std::int32_t)p_Height, p_Name.data(), nullptr, nullptr);
    if (windowHandle == nullptr) {
        std::cout << "Failed to create requested window" << std::endl;
    }

    glfwSetFramebufferSizeCallback(windowHandle, [](GLFWwindow* window, int new_width, int new_height) {
        GetInstance().GlobalWindowResizeCallback(window, new_width, new_height);
    });

    return this->RegisterWindow(windowHandle, Resolution { p_Width, p_Height });
}

bool WindowingSystem::HasOpenWindows() const
{
    for (size_t i = 0; i < this->m_Windows.size(); ++i) {
        if (this->m_WindowIsOpen[i])
            return true;
    }
    return false;
}

std::vector<WindowHandle> WindowingSystem::GetActiveWindowList()
{
    std::vector<WindowHandle> result;
    result.reserve(this->m_Windows.size());
    for (size_t i = 0; i < this->m_Windows.size(); ++i) {
        if (this->m_WindowIsOpen[i] == false)
            continue;

        result.push_back(WindowHandle {
            .Handle = i,
            .Generation = this->m_HandleGenerations[i],
        });
    }
    return result;
}

Resolution WindowingSystem::GetWindowResolution(WindowHandle p_Handle) const
{
    assert(p_Handle.Handle < this->m_Windows.size() && p_Handle.Generation == this->m_HandleGenerations[p_Handle.Handle]);
    return this->m_WindowResolutions[p_Handle.Handle];
}

bool WindowingSystem::IsWindowOpen(WindowHandle p_WindowHandle) const
{
    // assert(window_handle.generation == this->m_handle_generations[window_handle.handle]);
    return !glfwWindowShouldClose(this->m_Windows[p_WindowHandle.Handle]);
}

bool WindowingSystem::IsWindowFocused(WindowHandle p_WindowHandle) const
{
    return glfwGetWindowAttrib(this->m_Windows[p_WindowHandle.Handle], GLFW_FOCUSED) == GLFW_TRUE;
}

void WindowingSystem::RegisterWindowResizeCallback(WindowHandle p_Handle, std::function<void(std::uint32_t, std::uint32_t)> p_Callback)
{
    this->m_ResizeCallbacks[p_Handle.Handle].emplace_back(std::move(p_Callback));
}

WindowHandleNativeType WindowingSystem::GetNativeWindowHandle(WindowHandle p_Handle) const
{
    GLFWwindow* windowHandle = this->m_Windows[p_Handle.Handle];
#ifdef _WIN32
    return (FWD_HWND)glfwGetWin32Window(windowHandle);
#elifdef __APPLE__
    return glfwGetCocoaWindow(windowHandle);
#elifdef __linux__
    if (glfwGetWaylandDisplay()) {
        return glfwGetWaylandWindow(windowHandle);
    } else {
        return (void*)glfwGetX11Window(windowHandle);
    }
#endif
}

}
