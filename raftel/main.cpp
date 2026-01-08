#include <cassert>

#include "Rendering/API/DX12/DX12Renderer.h"
#include "Rendering/API/Vulkan/vulkan.h"
#include "Windowing/Window.h"
#include "logger.h"

#include <iostream>

using namespace raftel;

int main()
{
    auto& logger = logger::create_logger();
    WindowingSystem& windowing_system = WindowingSystem::get_instance();
    WindowHandle first_window = windowing_system.create_window("test_window", 1920, 1080);

    init_vulkan();

#ifdef _WIN32
    dx12::DX12Renderer renderer(first_window);
#endif

    while (windowing_system.has_open_windows()) {
        windowing_system.update();
        auto handles = windowing_system.get_active_window_list();
        for (auto& window : handles) {
            if (!windowing_system.is_window_open(window))
                continue;
        }
    }

    return 0;
}
