#include <iostream>

import GLFWInstance;
import Window;

int
main()
{
  raftel::Window window("Raftel", 1920, 1080);
  raftel::GLSurface surface = window.create_gl_surface();
  surface.make_current_context();

  while (window.is_open()) {
    window.update();
    surface.swap_buffers();
    surface.clear();
  }

  return 0;
}
