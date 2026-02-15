#include "DX12Renderer.h"

#include <Rendering/API/Windows/DirectX/DXGI/dxgi.h>
#include <Windowing/Window.h>
#include <logger.h>

#include <cassert>
#include <cstdint>
#include <d3d12sdklayers.h>

namespace raftel::dx12 {

std::vector<AdapterInfo> GetDeviceList()
{
    std::vector<AdapterInfo> adaptors;

    dxgi::DXGIFactory& factory = dxgi::DXGIFactory::GetFactory();

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

void dump_debug_messages(ComPtr<ID3D12InfoQueue> info_queue)
{
    assert(info_queue);

    const std::uint64_t messsage_count = info_queue->GetNumStoredMessages();
    for (size_t i = 0; i < messsage_count; ++i) {
        size_t message_size = 0;
        WIN_CALL(info_queue->GetMessage(i, nullptr, &message_size), "Failed to get D3D12 debug message size");

        D3D12_MESSAGE* message = (D3D12_MESSAGE*)malloc(message_size);
        WIN_CALL(info_queue->GetMessage(i, message, &message_size), "Failed to get D3D12 debug message");

        LOG_INFO("{}", message->pDescription);
    }
}

DX12Renderer::DX12Renderer(WindowHandle window, IDXGIAdapter4* gpuAdapter)
{
    WIN_CALL(D3D12GetDebugInterface(IID_PPV_ARGS(&this->m_debug_controller)),
        "Failed to load D3D12 Debug Controller");
    m_debug_controller->EnableDebugLayer();
    m_debug_controller->SetEnableGPUBasedValidation(TRUE);
    LOG_INFO("D3D12 Debug Controller Loaded & Activated");

    WIN_CALL(D3D12CreateDevice(gpuAdapter, D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&this->m_device)), "Failed to create D3D12 device");
    LOG_SUCCESS("Created D3D12 Device");

    ComPtr<ID3D12InfoQueue1> debug_info_queue = nullptr;
    WIN_CALL(this->m_device.As(&debug_info_queue), "Failed to obtain D3D12 Debug Info Queue");
    LOG_SUCCESS("Obtained D3D12 Debug Info Queue");
    dump_debug_messages(debug_info_queue);

    const D3D12_COMMAND_QUEUE_DESC queue_desc = {
        .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
        .Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
        .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
        .NodeMask = 0,
    };
    WIN_CALL(m_device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&this->m_graphics_command_queue)), "Failed to fetch D3D12 Graphics Queue");
    LOG_SUCCESS("Obtained D3D12 Graphics Queue");

    dxgi::DXGIFactory& factory = dxgi::DXGIFactory::GetFactory();
    this->m_swapchain = factory.CreateSwapchain(this->m_graphics_command_queue.Get(), window);
    LOG_SUCCESS("D3D12 Swapchain Created");

    ComPtr<ID3D12DescriptorHeap> rtv_descriptor_heap = nullptr;
    ComPtr<ID3D12DescriptorHeap> srv_descriptor_heap = nullptr;
    ComPtr<ID3D12DescriptorHeap> dsv_descriptor_heap = nullptr;
    D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {
        .Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
        .NumDescriptors = 99,
        .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
        .NodeMask = 0,
    };
    WIN_CALL(m_device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&rtv_descriptor_heap)), "Failed to create RTV Heap");
    heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    WIN_CALL(m_device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&dsv_descriptor_heap)), "Failed to create DSV Heap");
    heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    WIN_CALL(m_device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&srv_descriptor_heap)), "Failed to create SRV/CBV/UAV Heap");
    LOG_SUCCESS("Global D3D12 Descriptor Heaps Created");

    // unsigned int rtv_descriptor_handle_size = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    // unsigned int srv_descriptor_handle_size = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    // unsigned int dsv_descriptor_handle_size = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

}
