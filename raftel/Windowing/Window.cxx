module;

#include "GLFW.h"
#include <functional>
#include <iostream>
#include <unordered_map>
#include <vector>

import GLFWInstance;
import GLSurface;

export module Window;
namespace raftel {

export class Window;
DEFINE_HANDLE(Window)

export class WindowRegistry {
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

export class Window {
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

/* ================================================================ */

WindowRegistry WindowRegistry::m_instance = WindowRegistry();

void WindowRegistry::register_window(WindowHandle window)
{
    m_window_registry.insert({ window->get_window_handle(), window });
}

WindowHandle WindowRegistry::operator[](GLFWwindowHandle window) const
{
    return this->m_window_registry.at(window);
}

Window::Window(const char* title, uint32_t width, uint32_t height)
{
    raftel::GLFWInstance& instance = GLFWInstance::Instance();

    this->m_window_handle = glfwCreateWindow(width, height, title, nullptr, nullptr);

    WindowRegistry::get_instance().register_window(this);

    glfwSetFramebufferSizeCallback(
        this->m_window_handle,
        [](GLFWwindowHandle window, int new_width, int new_height) {
            WindowRegistry::get_instance()[window]->invoke_resize_callbacks(new_width, new_height);
        });
}

Window::~Window()
{
    glfwDestroyWindow(this->m_window_handle);
}

void Window::update()
{
    glfwPollEvents();
}

bool Window::is_open() const
{
    return !glfwWindowShouldClose(this->m_window_handle);
}

GLSurface Window::create_gl_surface() const
{
    return { this->m_window_handle };
}

GLFWwindowHandle Window::get_window_handle() const
{
    return this->m_window_handle;
}

void Window::register_resize_callback(const std::function<void(int, int)>& callback)
{
    this->m_resize_callbacks.push_back(std::move(callback));
}

void Window::invoke_resize_callbacks(int width, int height)
{
    for (int i = 0; i < this->m_resize_callbacks.size(); ++i) {
        this->m_resize_callbacks[i](width, height);
    }
}

}
