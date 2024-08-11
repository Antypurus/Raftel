#include <iostream>

#include <GLFW/glfw3.h>

import GLFWInstance;
import Window;

int main() {
  raftel::GLFWInstance::Instance();
  raftel::Window window("Raftel", 1920, 1080);

  while(window.IsOpen()) {
    window.Update();
  }

  std::cout << "iostream" << std::endl;

  return 0;
}
