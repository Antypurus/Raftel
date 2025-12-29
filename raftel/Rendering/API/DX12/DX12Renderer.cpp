#include "DX12Renderer.h"

#include <dxgi1_5.h>
#include <iostream>
#include <logger.h>

namespace raftel::dx12 {

DX12Renderer::DX12Renderer(HWND window_handle)
{
    WIN_CALL(D3D12GetDebugInterface(IID_PPV_ARGS(&this->m_debug_controller)),
        "Failed to load D3D12 Debug Controller");
    m_debug_controller->EnableDebugLayer();
    m_debug_controller->SetEnableGPUBasedValidation(TRUE);
    LOG_INFO("D3D12 Debug Controller Loaded & Activated");

    WIN_CALL(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&this->m_dxgi_factory)),
        "Failed to create DXGI Factory");
    LOG_INFO("DXGI Factory created");

    unsigned int adapter_index = 0;
    IDXGIAdapter1* adapter = nullptr;
    while (this->m_dxgi_factory->EnumAdapters1(adapter_index++, &adapter) == S_OK) {
        IDXGIAdapter4* interface_adapter = nullptr;
        WIN_CALL(adapter->QueryInterface(IID_PPV_ARGS(&interface_adapter)),
            "Failed to move factory interface from version 1 to version 4");

        DXGI_ADAPTER_DESC3 adapter_desc;
        interface_adapter->GetDesc3(&adapter_desc);
        std::wcout << adapter_index - 1 << ":" << adapter_desc.Description << std::endl;

        interface_adapter->Release();
    }

    ComPtr<ID3D12Device14> device;
    WIN_CALL(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&device)), "Failed to create D3D12 device");
    LOG_SUCCESS("Created D3D12 Device");

    ComPtr<ID3D12CommandQueue> command_queue;
    D3D12_COMMAND_QUEUE_DESC queue_desc = {
        .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
        .Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
        .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
        .NodeMask = 0,
    };
    WIN_CALL(device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&command_queue)), "Failed to fetch D3D12 Graphics Queue");
    LOG_SUCCESS("Obtained D3D12 Graphics Queue");

    DXGI_SWAP_CHAIN_DESC1 swapchain_desc = {

    };
    ComPtr<IDXGISwapChain1> swapchain = nullptr;
    WIN_CALL(this->m_dxgi_factory->CreateSwapChainForHwnd(device.Get(), window_handle, &swapchain_desc, nullptr, nullptr, swapchain.GetAddressOf()), "Failed to create D3D12 rendering swapchain");
    LOG_SUCCESS("D3D12 Swapchain Created")
}

}
