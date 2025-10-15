#include <iostream>

#include "Windowing/Window.h"

using namespace raftel;

int main()
{
    WindowingSystem& windowing_system = WindowingSystem::get_instance();
    WindowHandle first_window = windowing_system.create_window("test_window", 1920, 1080);
    WindowHandle second_window = windowing_system.create_window("test_window 2", 1280, 720);

    while (windowing_system.has_open_windows()) {
        windowing_system.update();
        auto handles = windowing_system.get_active_window_list();
        for (auto& window : handles) {
            if (!windowing_system.is_window_open(window))
                continue;

            windowing_system.make_window_current_context(window);
            windowing_system.swap_window_framebuffers(window);

            auto focused = windowing_system.is_window_focused(window);
            if (focused) {
                glClearColor(0.2f, 0.3f, 0.0f, 1.0f);
            } else {
                glClearColor(0.9f, 0.3f, 0.0f, 1.0f);
            }
            glClear(GL_COLOR_BUFFER_BIT);
        }
    }

    return 0;
}
