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
};

export class Window {
private:
    GLFWwindowHandle m_window_handle = nullptr;
    std::vector<std::function<void(int, int)>> resize_callbacks;

public:
    Window(const char* title, uint32_t width, uint32_t height);
    ~Window();

    void update();
    bool is_open() const;
    GLSurface create_gl_surface() const;
    GLFWwindowHandle get_window_handle() const;
};

} // namespace raftel

namespace raftel {

WindowRegistry WindowRegistry::m_instance = WindowRegistry();

void WindowRegistry::register_window(WindowHandle window)
{
    m_window_registry.insert({ window->get_window_handle(), window });
}

Window::Window(const char* title, uint32_t width, uint32_t height)
{
    raftel::GLFWInstance& instance = GLFWInstance::Instance();

    this->m_window_handle = glfwCreateWindow(width, height, title, nullptr, nullptr);

    WindowRegistry::get_instance().register_window(this);

    glfwSetFramebufferSizeCallback(
        this->m_window_handle,
        [](GLFWwindowHandle window, int new_width, int new_height) {
            std::cout << "Resized to [" << new_width << "x" << new_height << "]\n";
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

}
