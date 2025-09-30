#include <iostream>

#include "Windowing/Window.h"

using namespace raftel;

int main()
{
    WindowingSystem& windowing_system = WindowingSystem::get_instance();
    int window = windowing_system.create_window("test_window", 1920, 1080);
    windowing_system.make_window_current_context(window);

    while (windowing_system.window_is_open(window)) {
        windowing_system.update();
        windowing_system.swap_window_framebuffers(window);

        glClearColor(0.2f, 0.3f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    return 0;
}
