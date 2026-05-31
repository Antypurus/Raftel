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
    std::uint32_t width;
    std::uint32_t height;
};

struct WindowHandle {
    size_t handle;
    size_t generation;
};

class WindowingSystem {
private:
    static WindowingSystem s_instance;

private:
    // handle management
    std::vector<GLFWwindow*> m_windows;
    std::vector<size_t> m_handle_generations;

    // window metadata
    std::vector<Resolution> m_window_resolutions;
    std::vector<bool> m_window_is_open;
    std::vector<std::vector<std::function<void(std::uint32_t, std::uint32_t)>>> m_resize_callbacks;

public:
    static WindowingSystem& GetInstance();
    void Update();
    WindowHandle CreateWindow(std::string_view name, std::uint32_t width, std::uint32_t height);
    std::vector<WindowHandle> GetActiveWindowList();
    bool HasOpenWindows() const;
    bool IsWindowOpen(WindowHandle handle) const;
    bool IsWindowFocused(WindowHandle handle) const;
    void RegisterWindowResizeCallback(WindowHandle handle, std::function<void(std::uint32_t, std::uint32_t)> callback);
    WindowHandleNativeType GetNativeWindowHandle(WindowHandle handle) const;
    Resolution GetWindowResolution(WindowHandle handle) const;

private:
    WindowingSystem();
    ~WindowingSystem();

    void InitGLFW() const;
    void GlobalWindowResizeCallback(GLFWwindow* window_handle, int new_width, int new_height);
    WindowHandle RegisterWindow(GLFWwindow* window_handle, Resolution initial_resolution);
    void RemoveWindow(size_t index);
};

}
