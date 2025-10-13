#include <iostream>

#include "Windowing/Window.h"

using namespace raftel;

int main()
{
    WindowingSystem& windowing_system = WindowingSystem::get_instance();
    WindowHandle window = windowing_system.create_window("test_window", 1920, 1080);
    windowing_system.make_window_current_context(window);

    while (windowing_system.has_open_windows()) {
        windowing_system.update();
        windowing_system.swap_window_framebuffers(window);

        auto focused = windowing_system.is_window_focused(window);
        if (focused) {
            std::cout << "Window is focused" << std::endl;
        } else {
            std::cout << "Window is not focused" << std::endl;
        }

        glClearColor(0.2f, 0.3f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    return 0;
}
