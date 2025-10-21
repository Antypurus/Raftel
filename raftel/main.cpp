#include <iostream>

#include <fstream>
#include <string>

#include "Windowing/Window.h"

using namespace raftel;

std::string load_shader(std::string_view path)
{
    std::ifstream shader(path.data(), std::ios::in);

    // determine file size
    shader.seekg(0, std::ios::end);
    size_t size = shader.tellg();
    shader.seekg(0, std::ios::beg);

    std::string shader_code(size, '\0');
    shader.read(&shader_code[0], size);
    shader.close();

    return shader_code;
}

int main()
{
    WindowingSystem& windowing_system = WindowingSystem::get_instance();
    WindowHandle first_window = windowing_system.create_window("test_window", 1920, 1080);

    std::string vertex_shader = load_shader("shaders/basic/vert.glsl");
    std::cout << vertex_shader << std::endl;

    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.0f, 0.5f, 0.0f
    };

    unsigned int vbo_handle = 0;
    glGenBuffers(1, &vbo_handle);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_handle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    while (windowing_system.has_open_windows()) {
        windowing_system.update();
        auto handles = windowing_system.get_active_window_list();
        for (auto& window : handles) {
            if (!windowing_system.is_window_open(window))
                continue;
            windowing_system.make_window_current_context(window);
            windowing_system.swap_window_framebuffers(window);

            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
        }
    }

    return 0;
}
