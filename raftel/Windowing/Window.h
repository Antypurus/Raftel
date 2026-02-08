#pragma once

#include <functional>
#include <string_view>
#include <vector>

#if defined(_WIN32)
    #define GLFW_EXPOSE_NATIVE_WIN32
    #define WINDOW_HANDLE_NATIVE_TYPE HWND
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

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"
using WindowHandleNativeType = WINDOW_HANDLE_NATIVE_TYPE;

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
    static WindowingSystem& get_instance();
    void update();
    WindowHandle create_window(std::string_view name, std::uint32_t width, std::uint32_t height);
    std::vector<WindowHandle> get_active_window_list();
    bool has_open_windows() const;
    bool is_window_open(WindowHandle handle) const;
    bool is_window_focused(WindowHandle handle) const;
    void register_window_resize_callback(WindowHandle handle, std::function<void(std::uint32_t, std::uint32_t)> callback);
    WindowHandleNativeType get_native_window_handle(WindowHandle handle) const;
    Resolution get_window_resolution(WindowHandle handle) const;

private:
    WindowingSystem();
    ~WindowingSystem();

    void init_glfw() const;
    void global_window_resize_callback(GLFWwindow* window_handle, int new_width, int new_height);
    WindowHandle register_window(GLFWwindow* window_handle, Resolution initial_resolution);
    void remove_window(size_t index);
};

}
