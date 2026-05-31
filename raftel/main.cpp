#ifdef _WIN32
    #include <Rendering/API/Windows/DirectX/DX11/DX11.h>
    #include <Rendering/API/Windows/DirectX/DX12/DX12Renderer.h>
#endif
#ifdef __APPLE__
    #include <Rendering/API/MacOS/Metal/Metal.h>
#endif

#include <Rendering/API/Vulkan/vulkan.h>
#include <Windowing/Window.h>
#include <core/filesystem/filesystem.h>
#include <core/filesystem/parsers/json/json.h>
#include <core/logger.h>

using namespace raftel;

int main()
{
    logger::create_logger();

    auto fileContents = raftel::filesystem::ReadFile("README.md");
    LOG_INFO("{}", std::string_view((char*)fileContents.data(), fileContents.size()));
    //return 0;

    WindowingSystem& windowing_system = WindowingSystem::GetInstance();
    WindowHandle first_window = windowing_system.CreateWindow("test_window", 1920, 1080);

    // init_vulkan();

#ifdef __APPLE__
    init_metal(first_window);
#endif

#if defined(_WIN32) && 1
    #if 1
    auto device = dx11::GPUDevice::CreateDevice();
    auto swapchain = device.CreateSwapchain(first_window);
    windowing_system.RegisterWindowResizeCallback(first_window, [&swapchain](std::uint32_t width, std::uint32_t height) { swapchain.RegisterResize(width, height); });

    auto vertex_shader = device.CompileVertexShader(L"shaders/basic/hlsl/basic.hlsl");
    auto pixel_shader = device.CompilePixelShader(L"shaders/basic/hlsl/basic.hlsl");
    auto vertex_buffer = device.CreateVertexBuffer({
                                                       0.0f, 0.5f, 0.0f, // top left
                                                       0.5f, -0.5f, 0.0f, // top right
                                                       -0.5f, -0.5f, 0.0f // tip
                                                   },
        vertex_shader.value());
    auto index_buffer = device.CreateIndexBuffer({ 0, 1, 2 });

    device.Bind(vertex_shader.value());
    device.Bind(pixel_shader.value());
    device.Bind(vertex_buffer);
    device.Bind(index_buffer);

    while (windowing_system.HasOpenWindows()) {
        windowing_system.Update();
        auto handles = windowing_system.GetActiveWindowList();
        for (auto& window : handles) {
            if (!windowing_system.IsWindowOpen(window))
                continue;

            device.Clear(swapchain);

            device.Bind(swapchain);
            device.DrawTriangles(3);
            swapchain.Present();

            device.DumpErrorMessages();
        }
    }
    #else
    dx12::DX12Renderer renderer(first_window);
    #endif
#endif

    return 0;
}
