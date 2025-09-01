module;

#define GLFW_INCLUDE_NONE

#include "../handle.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <iostream>

import GLFWInstance;

DEFINE_HANDLE(GLFWwindow)

export module Window;
namespace raftel {

export class GLSurface
{
private:
  GLFWwindowHandle m_window_handle = nullptr;

public:
  GLSurface(GLFWwindowHandle window_handle)
    : m_window_handle(window_handle)
  {
    if (GL_ARB_compute_shader) {
      std::cout << "compute shaders supported via extension" << std::endl;
    }
  }

  void make_current_context() const
  {
    glfwMakeContextCurrent(m_window_handle);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
      std::cout << "Failed to initialize GLAD" << std::endl;
      return;
    }
  }

  void clear() const
  {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
  }
};

export class Window
{
private:
  GLFWwindowHandle m_window_handle = nullptr;

public:
  Window(const char* title, uint32_t width, uint32_t height)
  {
    raftel::GLFWInstance& instance = GLFWInstance::Instance();

    this->m_window_handle =
      glfwCreateWindow(width, height, title, nullptr, nullptr);
  }

  ~Window() { glfwDestroyWindow(this->m_window_handle); }

  void update()
  {
    glfwSwapBuffers(this->m_window_handle);
    glfwPollEvents();
  }

  bool is_open() const { return !glfwWindowShouldClose(this->m_window_handle); }

  GLSurface create_gl_surface() const { return { this->m_window_handle }; }
};

} // namespace raftel
