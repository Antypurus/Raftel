#pragma once

#include <functional>
#include <unordered_map>
#include <vector>

#include "GLFWInstance.h"
#include "Surfaces/GLSurface.h"

namespace raftel {

class Window;
DEFINE_HANDLE(Window)

class WindowRegistry {
private:
    static WindowRegistry m_instance;
    std::unordered_map<GLFWwindowHandle, WindowHandle> m_window_registry;

private:
    WindowRegistry() = default;

public:
    static WindowRegistry& get_instance() { return m_instance; }

    void register_window(WindowHandle window);
    WindowHandle operator[](GLFWwindowHandle window) const;
};

class Window {
private:
    GLFWwindowHandle m_window_handle = nullptr;
    std::vector<std::function<void(int, int)>> m_resize_callbacks;

public:
    Window(const char* title, uint32_t width, uint32_t height);
    ~Window();

    bool is_open() const;
    GLFWwindowHandle get_window_handle() const;

    void update();
    GLSurface create_gl_surface() const;
    void register_resize_callback(const std::function<void(int, int)>& callback);
    void invoke_resize_callbacks(int width, int height);
};

}
