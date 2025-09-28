#include <iostream>

#include "Windowing/Window.h"

int main()
{
    raftel::Window window("Raftel", 1920, 1080);
    raftel::GLSurface surface = window.create_gl_surface();
    surface.make_current_context();

    window.register_resize_callback([](int width, int heigh) {
        std::cout << "Resized window to " << width << "x" << heigh << std::endl;
    });

    while (window.is_open()) {
        window.update();
        surface.swap_buffers();
        surface.clear();
    }

    return 0;
}
