#include "DX12Renderer.h"

#include <Rendering/API/Windows/DirectX/DXGI/dxgi.h>
#include <Windowing/Window.h>
#include <core/logger.h>

#include <cassert>
#include <cstdint>
#include <d3d12sdklayers.h>

namespace raftel::dx12 {

std::vector<AdapterInfo> GetDeviceList()
{
    std::vector<AdapterInfo> adaptors;

    dxgi::DXGIFactory& factory = dxgi::DXGIFactory::GetFactory();

    unsigned int adapterIndex = 0;
    IDXGIAdapter1* adapter = nullptr;
    while (factory->EnumAdapters1(adapterIndex++, &adapter) == S_OK) {
        IDXGIAdapter4* interfaceAdapter = nullptr;
        WIN_CALL(adapter->QueryInterface(IID_PPV_ARGS(&interfaceAdapter)), "Failed to upcast device adaptor");

        DXGI_ADAPTER_DESC3 adapter_desc;
        WIN_CALL(interfaceAdapter->GetDesc3(&adapter_desc), "Failed to get device adaptor information");

        adaptors.push_back({
            .Adapter = interfaceAdapter,
            .Vendor = (AdapterVendor)adapter_desc.VendorId,
            .Name = adapter_desc.Description,
            .DedicatedMemoryBytes = adapter_desc.DedicatedVideoMemory,
        });

        interfaceAdapter->Release();
        adapter->Release();
    }

    return adaptors;
}

void dump_debug_messages(ComPtr<ID3D12InfoQueue> p_InfoQueue)
{
    assert(p_InfoQueue);

    const std::uint64_t messsageCount = p_InfoQueue->GetNumStoredMessages();
    for (size_t i = 0; i < messsageCount; ++i) {
        size_t messageSize = 0;
        WIN_CALL(p_InfoQueue->GetMessage(i, nullptr, &messageSize), "Failed to get D3D12 debug message size");

        D3D12_MESSAGE* message = (D3D12_MESSAGE*)malloc(messageSize);
        WIN_CALL(p_InfoQueue->GetMessage(i, message, &messageSize), "Failed to get D3D12 debug message");

        LOG_INFO("{}", message->pDescription);
    }
}

DX12Renderer::DX12Renderer(WindowHandle p_Window, IDXGIAdapter4* p_GPUAdapter)
{
    WIN_CALL(D3D12GetDebugInterface(IID_PPV_ARGS(&this->m_DebugController)),
        "Failed to load D3D12 Debug Controller");
    m_DebugController->EnableDebugLayer();
    m_DebugController->SetEnableGPUBasedValidation(TRUE);
    LOG_INFO("D3D12 Debug Controller Loaded & Activated");

    WIN_CALL(D3D12CreateDevice(p_GPUAdapter, D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&this->m_Device)), "Failed to create D3D12 device");
    LOG_SUCCESS("Created D3D12 Device");

    ComPtr<ID3D12InfoQueue1> debugInfoQueue = nullptr;
    WIN_CALL(this->m_Device.As(&debugInfoQueue), "Failed to obtain D3D12 Debug Info Queue");
    LOG_SUCCESS("Obtained D3D12 Debug Info Queue");
    dump_debug_messages(debugInfoQueue);

    const D3D12_COMMAND_QUEUE_DESC queueDesc = {
        .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
        .Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
        .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
        .NodeMask = 0,
    };
    WIN_CALL(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&this->m_GraphicsCommandQueue)), "Failed to fetch D3D12 Graphics Queue");
    LOG_SUCCESS("Obtained D3D12 Graphics Queue");

    dxgi::DXGIFactory& factory = dxgi::DXGIFactory::GetFactory();
    this->m_Swapchain = factory.CreateSwapchain(this->m_GraphicsCommandQueue.Get(), p_Window);
    LOG_SUCCESS("D3D12 Swapchain Created");

    ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap = nullptr;
    ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap = nullptr;
    ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap = nullptr;
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
        .Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
        .NumDescriptors = 99,
        .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
        .NodeMask = 0,
    };
    WIN_CALL(m_Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvDescriptorHeap)), "Failed to create RTV Heap");
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    WIN_CALL(m_Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&dsvDescriptorHeap)), "Failed to create DSV Heap");
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    WIN_CALL(m_Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&srvDescriptorHeap)), "Failed to create SRV/CBV/UAV Heap");
    LOG_SUCCESS("Global D3D12 Descriptor Heaps Created");

    // unsigned int rtv_descriptor_handle_size = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    // unsigned int srv_descriptor_handle_size = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    // unsigned int dsv_descriptor_handle_size = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

}
