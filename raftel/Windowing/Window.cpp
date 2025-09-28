#include "Window.h"


namespace raftel {

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
