#pragma once

#include <string_view>
#include <vector>

#include "GLFW.h"

namespace raftel {

struct Resolution {
    size_t width;
    size_t height;
};

struct WindowHandle {
    size_t handle;
    size_t generation;
};

class WindowingSystem {
private:
    static WindowingSystem s_instance;

private:
    std::vector<GLFWwindow*> m_windows;
    std::vector<Resolution> m_window_resolutions;

public:
    static WindowingSystem& get_instance();
    void update();
    size_t create_window(std::string_view name, int width, int height);
    bool has_open_windows() const;
    bool is_window_open(int window_handle) const;
    bool is_window_focused(int window_handle) const;
    void make_window_current_context(int window_handle) const;
    void swap_window_framebuffers(int window_handle) const;

private:
    WindowingSystem();
    ~WindowingSystem();

    void init_glfw() const;
    void load_opengl() const;
    void global_window_resize_callback(GLFWwindow* window_handle, int new_width, int new_height);
};

}
