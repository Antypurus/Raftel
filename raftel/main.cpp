#include <iostream>

#include <GLFW/glfw3.h>

import GLFWInstance;

int main() {
  raftel::GLFWInstance::Instance();

  auto window = glfwCreateWindow(1280, 720, "Onto Raftael", nullptr, nullptr);
  if (!window)
    return -1;

  glfwMakeContextCurrent(window);

  while (!glfwWindowShouldClose(window)) {
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  std::cout << "iostream" << std::endl;

  glfwDestroyWindow(window);
  return 0;
}
