#include "DX11.h"
#include "Rendering/API/DirectX/DXGI/dxgi.h"

#include <Rendering/API/DirectX/DX12/DX12Renderer.h>
#include <Windowing/Window.h>
#include <combaseapi.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <dxgi1_5.h>
#include <dxgiformat.h>
#include <dxgitype.h>
#include <logger.h>

#include <assert.h>
#include <cstdint>
#include <iostream>

#ifdef _WIN32
namespace raftel::dx11 {

GPUDevice GPUDevice::CreateDevice()
{
    std::vector<dx12::AdapterInfo> adaptors = dx12::GetDeviceList();
    assert(adaptors.size() > 0);

    IDXGIAdapter* p_adapter = adaptors[0].adapter.Get();
    std::wcout << "Selected Adapter: " << adaptors[0].name << std::endl;

    D3D_FEATURE_LEVEL features_levels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_11_1
    };
    D3D_FEATURE_LEVEL selected_feature_level;

    ComPtr<ID3D11Device> device = nullptr;
    ComPtr<ID3D11DeviceContext> context = nullptr;
    WIN_CALL(D3D11CreateDevice(
                 p_adapter,
                 D3D_DRIVER_TYPE_UNKNOWN,
                 nullptr,
                 D3D11_CREATE_DEVICE_DEBUG,
                 features_levels,
                 _countof(features_levels),
                 D3D11_SDK_VERSION,
                 device.GetAddressOf(),
                 &selected_feature_level,
                 context.GetAddressOf()),
        "Failed to create D3D11 Device");

    ComPtr<ID3D11Device5> final_dev = nullptr;
    WIN_CALL(device->QueryInterface(IID_PPV_ARGS(&final_dev)), "Failed to upcast D3D11 device to version 5");
    return { final_dev };
}

GPUDevice::GPUDevice(ComPtr<ID3D11Device5> device)
    : m_device(device)
{
}

ID3D11Device5* GPUDevice::operator->()
{
    return this->m_device.Get();
}

ID3D11Device5* GPUDevice::get()
{
    return this->m_device.Get();
}

std::vector<std::string> GPUDevice::GetErrorMessages() const
{
    std::vector<std::string> error_messages;

    ComPtr<ID3D11InfoQueue> info_queue = nullptr;
    WIN_CALL(this->m_device->QueryInterface(IID_PPV_ARGS(&info_queue)), "Failed to obtain D3D11 Info-Queue");
    const std::uint64_t message_count = info_queue->GetNumStoredMessages();
    for (size_t i = 0; i < message_count; ++i) {
        size_t message_length = 0;
        WIN_CALL(info_queue->GetMessage(i, nullptr, &message_length), "Failed to get error message length");

        D3D11_MESSAGE* message = (D3D11_MESSAGE*)malloc(message_length);
        WIN_CALL(info_queue->GetMessage(i, message, &message_length), "Failed to get error message");

        error_messages.emplace_back(message->pDescription, message->DescriptionByteLength);
        free(message);
    }
    info_queue->ClearStoredMessages();
    return error_messages;
}

void GPUDevice::DumpErrorMessages() const
{
    std::vector<std::string> error_messages = this->GetErrorMessages();
    for (const auto& message : error_messages) {
        LOG_ERROR("{}", message);
    }
}

// TODO(Tiago):move this method into the factory "class", we can supply it with either a dx11 or dx12 device
// so it does not make sense to bind it to any of those devices
ComPtr<IDXGISwapChain4> GPUDevice::CreateSwapchain(WindowHandle window, dxgi::ResourceFormat format)
{
    WindowingSystem& window_system = WindowingSystem::get_instance();
    HWND window_handle = window_system.get_native_window_handle(window);
    Resolution window_resolution = window_system.get_window_resolution(window);

    ComPtr<IDXGISwapChain4> swapchain = nullptr;
    auto factory = dxgi::GetDXGIFactory();

    ComPtr<IDXGISwapChain1> intermediate_swapchain = nullptr;
    const DXGI_SWAP_CHAIN_DESC1 swapchain_desc = {
        .Width = window_resolution.width,
        .Height = window_resolution.height,
        .Format = (DXGI_FORMAT)format,
        .Stereo = false,
        .SampleDesc = {
            .Count = 1,
            .Quality = 0,
        },
        .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
        .BufferCount = 2,
        .Scaling = DXGI_SCALING_STRETCH,
        .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
        .AlphaMode = DXGI_ALPHA_MODE_IGNORE,
        .Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING,
    };
    const DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreen_desc = {
        .RefreshRate = {
            .Numerator = 144,
            .Denominator = 1,
        },
        .ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
        .Scaling = DXGI_MODE_SCALING_STRETCHED,
        .Windowed = true,
    };
    DXGI_CALL(factory->CreateSwapChainForHwnd(
                  this->get(),
                  WindowingSystem::get_instance().get_native_window_handle(window),
                  &swapchain_desc,
                  &fullscreen_desc,
                  nullptr,
                  intermediate_swapchain.GetAddressOf()),
        "Failed to create swapchain");

    WIN_CALL(intermediate_swapchain->QueryInterface(IID_PPV_ARGS(&swapchain)), "Failed to upcast DXGI Swapchain to version 4");
    return swapchain;
}

void init_d3d11(WindowHandle window)
{
    auto device = GPUDevice::CreateDevice();

    ComPtr<ID3D11Debug> debug = nullptr;
    WIN_CALL(device->QueryInterface(IID_PPV_ARGS(&debug)), "Failed to query debug interface");

    auto swapchain = device.CreateSwapchain(window);

    // DX11_CALL(device->CreateBuffer(nullptr, nullptr, nullptr), device, "Failed to create buffer");
}
}
#endif
