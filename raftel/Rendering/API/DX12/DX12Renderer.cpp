#include "DX12Renderer.h"
#include "Windowing/Window.h"

#include <combaseapi.h>
#include <dxgi1_6.h>
#include <iostream>
#include <logger.h>

namespace raftel::dx12 {

std::vector<AdapterInfo> DX12Renderer::GetDeviceList()
{
    std::vector<AdapterInfo> adaptors;

    ComPtr<IDXGIFactory7> factory = nullptr;
#ifndef NDEBUG
    unsigned int flags = DXGI_CREATE_FACTORY_DEBUG;
#else
    unsigned int flags = 0;
#endif
    WIN_CALL(CreateDXGIFactory2(flags, IID_PPV_ARGS(&factory)), "Failed to create temporary factory for device listing");

    unsigned int adapter_index = 0;
    IDXGIAdapter1* adapter = nullptr;
    while (factory->EnumAdapters1(adapter_index++, &adapter) == S_OK) {
        IDXGIAdapter4* interface_adapter = nullptr;
        WIN_CALL(adapter->QueryInterface(IID_PPV_ARGS(&interface_adapter)), "Failed to upcast device adaptor");

        DXGI_ADAPTER_DESC3 adapter_desc;
        WIN_CALL(interface_adapter->GetDesc3(&adapter_desc), "Failed to get device adaptor information");

        adaptors.push_back({
            .adapter = interface_adapter,
            .vendor = (AdapterVendor)adapter_desc.VendorId,
            .name = adapter_desc.Description,
            .dedicated_memory_bytes = adapter_desc.DedicatedVideoMemory,
        });

        interface_adapter->Release();
        adapter->Release();
    }

    return adaptors;
}

DX12Renderer::DX12Renderer(WindowHandle window, IDXGIAdapter4* gpuAdapter)
{
    WIN_CALL(D3D12GetDebugInterface(IID_PPV_ARGS(&this->m_debug_controller)),
        "Failed to load D3D12 Debug Controller");
    m_debug_controller->EnableDebugLayer();
    m_debug_controller->SetEnableGPUBasedValidation(TRUE);
    LOG_INFO("D3D12 Debug Controller Loaded & Activated");

    WIN_CALL(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&this->m_dxgi_factory)),
        "Failed to create DXGI Factory");
    LOG_INFO("DXGI Factory created");

    WIN_CALL(D3D12CreateDevice(gpuAdapter, D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&this->m_device)), "Failed to create D3D12 device");
    LOG_SUCCESS("Created D3D12 Device");

    const D3D12_COMMAND_QUEUE_DESC queue_desc = {
        .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
        .Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
        .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
        .NodeMask = 0,
    };
    WIN_CALL(m_device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&this->m_graphics_command_queue)), "Failed to fetch D3D12 Graphics Queue");
    LOG_SUCCESS("Obtained D3D12 Graphics Queue");

    const DXGI_SWAP_CHAIN_DESC1 swapchain_desc = {
        .Width = 1920,
        .Height = 1080,
        .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
        .Stereo = false,
        .SampleDesc = DXGI_SAMPLE_DESC {
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
    ComPtr<IDXGISwapChain1> intermediate_swapchain = nullptr;
    WIN_CALL(this->m_dxgi_factory->CreateSwapChainForHwnd(
                 this->m_graphics_command_queue.Get(),
                 WindowingSystem::get_instance().get_native_window_handle(window),
                 &swapchain_desc, nullptr, nullptr, intermediate_swapchain.GetAddressOf()),
        "Failed to create D3D12 rendering swapchain");
    WIN_CALL(intermediate_swapchain->QueryInterface(IID_PPV_ARGS(&this->m_swapchain)), "Failed to upcast to swapchain version 4");
    LOG_SUCCESS("D3D12 Swapchain Created")
}

}
