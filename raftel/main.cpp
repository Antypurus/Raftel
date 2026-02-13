#ifdef _WIN32
    #include <Rendering/API/Windows/DirectX/DX11/DX11.h>
    #include <Rendering/API/Windows/DirectX/DX12/DX12Renderer.h>
#endif
#ifdef __APPLE__
    #include <Rendering/API/MacOS/Metal/Metal.h>
#endif

#include <Rendering/API/Vulkan/vulkan.h>
#include <Windowing/Window.h>
#include <logger.h>

using namespace raftel;

int main()
{
    logger::create_logger();
    WindowingSystem& windowing_system = WindowingSystem::get_instance();
    WindowHandle first_window = windowing_system.create_window("test_window", 1920, 1080);

    // init_vulkan();

#ifdef __APPLE__
    init_metal();
#endif

#ifdef _WIN32
    #if 1
    auto device = dx11::GPUDevice::CreateDevice();
    auto swapchain = device.CreateSwapchain(first_window);
    windowing_system.register_window_resize_callback(first_window, [&swapchain](std::uint32_t width, std::uint32_t height) { swapchain.RegisterResize(width, height); });
    #else
        // dx12::DX12Renderer renderer(first_window);
    #endif
#endif

#if 1
    while (windowing_system.has_open_windows()) {
        windowing_system.update();
        auto handles = windowing_system.get_active_window_list();
        for (auto& window : handles) {
            if (!windowing_system.is_window_open(window))
                continue;

    #ifdef _WIN32
            swapchain.Present();
            device.Clear(swapchain);
    #endif
        }
    }
#endif

    return 0;
}
