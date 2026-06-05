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
    Logger::CreateLogger();

    auto fileContents = raftel::filesystem::ReadFile("README.md");
    LOG_INFO("{}", std::string_view((char*)fileContents.data(), fileContents.size()));

    WindowingSystem& windowingSystem = WindowingSystem::GetInstance();
    WindowHandle firstWindow = windowingSystem.CreateWindow("test_window", 1920, 1080);

    // init_vulkan();

#ifdef __APPLE__
    InitMetal(firstWindow);
#endif

#if defined(_WIN32) && 1
    #if 1
    auto device = dx11::GPUDevice::CreateDevice();
    auto swapchain = device.CreateSwapchain(firstWindow);
    windowingSystem.RegisterWindowResizeCallback(firstWindow, [&swapchain](std::uint32_t width, std::uint32_t height) { swapchain.RegisterResize(width, height); });

    auto vertexShader = device.CompileVertexShader(L"shaders/basic/hlsl/basic.hlsl");
    auto pixelShader = device.CompilePixelShader(L"shaders/basic/hlsl/basic.hlsl");
    auto vertexBuffer = device.CreateVertexBuffer({
                                                      0.0f, 0.5f, 0.0f, // top left
                                                      0.5f, -0.5f, 0.0f, // top right
                                                      -0.5f, -0.5f, 0.0f // tip
                                                  },
        vertexShader.value());
    auto indexBuffer = device.CreateIndexBuffer({ 0, 1, 2 });

    device.Bind(vertexShader.value());
    device.Bind(pixelShader.value());
    device.Bind(vertexBuffer);
    device.Bind(indexBuffer);

    while (windowingSystem.HasOpenWindows()) {
        windowingSystem.Update();
        auto handles = windowingSystem.GetActiveWindowList();
        for (auto& window : handles) {
            if (!windowingSystem.IsWindowOpen(window))
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
