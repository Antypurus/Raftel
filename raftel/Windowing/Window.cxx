module;

#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <iostream>

import GLFWInstance;

export module Window;
namespace raftel {

export class Window {
private:
  GLFWwindow *m_window_handle = nullptr;

public:
  Window(const char *title, uint32_t width, uint32_t height) {
    raftel::GLFWInstance &instance = GLFWInstance::Instance();

    this->m_window_handle =
        glfwCreateWindow(width, height, title, nullptr, nullptr);
    glfwMakeContextCurrent(this->m_window_handle);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
      std::cout << "Failed to initialize GLAD" << std::endl;
      return;
    }


    if(GL_ARB_compute_shader)
    {
        std::cout << "compute shaders supported via extension" << std::endl;
    }
  }

  ~Window() { glfwDestroyWindow(this->m_window_handle); }

  void Update() {
    glClearColor(0.4f, 0.3f, 0.4f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(this->m_window_handle);
    glfwPollEvents();
  }

  bool IsOpen() const { return !glfwWindowShouldClose(this->m_window_handle); }
};

} // namespace raftel
