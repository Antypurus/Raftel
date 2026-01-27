#pragma once

#include "Windowing/Window.h"
#include <Rendering/API/DX12/DX12.h>
#include <vector>

namespace raftel::dx12 {

using namespace Microsoft::WRL;

enum class AdapterVendor {
    Intel = 0x8086,
    AMD = 0x1002,
    NVidia = 0x10DE,
};

struct AdapterInfo {
    ComPtr<IDXGIAdapter4> adapter = nullptr;
    AdapterVendor vendor;
    std::wstring name;
    std::uint64_t dedicated_memory_bytes = 0;
};
std::vector<AdapterInfo> GetDeviceList();

struct DX12Renderer {
private:
    ComPtr<IDXGIFactory7> m_dxgi_factory;
    ComPtr<ID3D12Debug6> m_debug_controller;
    ComPtr<ID3D12Device14> m_device;
    ComPtr<IDXGISwapChain4> m_swapchain;
    ComPtr<ID3D12CommandQueue> m_graphics_command_queue;

public:
    DX12Renderer(WindowHandle window, IDXGIAdapter4* gpuAdapter = nullptr);
};

}
