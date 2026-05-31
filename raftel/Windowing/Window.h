#pragma once

#include <functional>
#include <string_view>
#include <vector>

#if defined(_WIN32)
    #define GLFW_EXPOSE_NATIVE_WIN32
    #define FWD_HWND int*
    #define WINDOW_HANDLE_NATIVE_TYPE FWD_HWND
#elif defined(__APPLE__)
    #define GLFW_EXPOSE_NATIVE_COCOA
    // #define WINDOW_HANDLE_NATIVE_TYPE NSWindow*
    #define WINDOW_HANDLE_NATIVE_TYPE void*
#elif defined(__linux__)
    #define GLFW_EXPOSE_NATIVE_X11
    #define GLFW_EXPOSE_NATIVE_WAYLAND
    #define WINDOW_HANDLE_NATIVE_TYPE void*
#else
    #define WINDOW_HANDLE_NATIVE_TYPE void*
#endif

using WindowHandleNativeType = WINDOW_HANDLE_NATIVE_TYPE;

struct GLFWwindow;

namespace raftel {

struct Resolution {
    std::uint32_t Width;
    std::uint32_t Height;
};

struct WindowHandle {
    size_t Handle;
    size_t Generation;
};

class WindowingSystem {
private:
    static WindowingSystem s_Instance;

private:
    // handle management
    std::vector<GLFWwindow*> m_Windows;
    std::vector<size_t> m_HandleGenerations;

    // window metadata
    std::vector<Resolution> m_WindowResolutions;
    std::vector<bool> m_WindowIsOpen;
    std::vector<std::vector<std::function<void(std::uint32_t, std::uint32_t)>>> m_ResizeCallbacks;

public:
    static WindowingSystem& GetInstance();
    void Update();
    WindowHandle CreateWindow(std::string_view Name, std::uint32_t Width, std::uint32_t Height);
    std::vector<WindowHandle> GetActiveWindowList();
    bool HasOpenWindows() const;
    bool IsWindowOpen(WindowHandle Handle) const;
    bool IsWindowFocused(WindowHandle Handle) const;
    void RegisterWindowResizeCallback(WindowHandle Handle, std::function<void(std::uint32_t, std::uint32_t)> Callback);
    WindowHandleNativeType GetNativeWindowHandle(WindowHandle Handle) const;
    Resolution GetWindowResolution(WindowHandle Handle) const;

private:
    WindowingSystem();
    ~WindowingSystem();

    void InitGLFW() const;
    void GlobalWindowResizeCallback(GLFWwindow* WindowHandle, int NewWidth, int NewHeight);
    WindowHandle RegisterWindow(GLFWwindow* WindowHandle, Resolution InitialResolution);
    void RemoveWindow(size_t Index);
};

}
