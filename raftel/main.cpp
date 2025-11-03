#include <iostream>

#include <cassert>
#include <fstream>
#include <string>

#include "Windowing/Window.h"
#include "glad/glad.h"

using namespace raftel;

enum class ShaderType {
    vertex = GL_VERTEX_SHADER,
    fragment = GL_FRAGMENT_SHADER,
    compute = GL_COMPUTE_SHADER,
};

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

unsigned int load_and_compile_shader(std::string_view path, ShaderType type)
{
    std::string shadertext = load_shader(path);
    unsigned int shader_handle = glCreateShader((int)type);

    const char* shader_to_compile[] = { shadertext.c_str() };
    glShaderSource(shader_handle, 1, shader_to_compile, nullptr);
    glCompileShader(shader_handle);

    int status = 0;
    glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        char log[1024];
        glGetShaderInfoLog(shader_handle, 1024, nullptr, log);
        std::cout << "Shader Compilation Failed\n"
                  << log << std::endl;
        return -1;
    }

    return shader_handle;
}

unsigned int create_shader_program(const std::vector<unsigned int> shaders)
{
    unsigned int shader_program_handle = glCreateProgram();
    for (unsigned int shader : shaders) {
        glAttachShader(shader_program_handle, shader);
    }
    glLinkProgram(shader_program_handle);

    int status = 0;
    glGetProgramiv(shader_program_handle, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        char log[1024];
        glGetProgramInfoLog(shader_program_handle, 1024, NULL, log);
        std::cout << "Shader Program Linking Failed\n"
                  << log << std::endl;
        return -1;
    }

    return shader_program_handle;
}

int main()
{
    WindowingSystem& windowing_system = WindowingSystem::get_instance();
    WindowHandle first_window = windowing_system.create_window("test_window", 1920, 1080);
    WindowHandle second_window = windowing_system.create_window("test_window_2", 1920, 1080);
    windowing_system.make_window_current_context(second_window);

    unsigned int vertex_shader_handle = load_and_compile_shader("shaders/basic/vert.glsl", ShaderType::vertex);
    unsigned int fragment_shader_handle = load_and_compile_shader("shaders/basic/frag.glsl", ShaderType::fragment);
    unsigned int basic_shader_program = create_shader_program({ vertex_shader_handle, fragment_shader_handle });
    glDeleteShader(vertex_shader_handle);
    glDeleteShader(fragment_shader_handle);

    unsigned int vao_handle;
    glGenVertexArrays(1, &vao_handle);
    glBindVertexArray(vao_handle);

    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.0f, 0.5f, 0.0f
    };

    unsigned int vbo_handle = 0;
    glGenBuffers(1, &vbo_handle);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_handle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    while (windowing_system.has_open_windows()) {
        windowing_system.update();
        auto handles = windowing_system.get_active_window_list();
        for (auto& window : handles) {
            if (!windowing_system.is_window_open(window))
                continue;
            windowing_system.make_window_current_context(window);
            windowing_system.swap_window_framebuffers(window);

            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glUseProgram(basic_shader_program);
            glBindVertexArray(vao_handle);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }
    }

    return 0;
}
