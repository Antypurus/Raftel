#include <iostream>

import GLFWInstance;
import Window;

int main() {
  raftel::Window window("Raftel", 1920, 1080);

  while (window.IsOpen()) {
    window.Update();
  }

  std::cout << "iostream" << std::endl;

  return 0;
}
