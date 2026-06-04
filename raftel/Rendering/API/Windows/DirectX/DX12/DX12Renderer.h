#pragma once

#include <Rendering/API/Windows/DirectX/DX12/DX12.h>
#include <Windowing/Window.h>

#include <vector>

namespace raftel::dx12 {

using namespace Microsoft::WRL;

enum class AdapterVendor {
    Intel = 0x8086,
    AMD = 0x1002,
    NVidia = 0x10DE,
};

struct AdapterInfo {
    ComPtr<IDXGIAdapter4> Adapter = nullptr;
    AdapterVendor Vendor;
    std::wstring Name;
    std::uint64_t DedicatedMemoryBytes = 0;
};
std::vector<AdapterInfo> GetDeviceList();

struct DX12Renderer {
private:
    ComPtr<ID3D12Debug6> m_DebugController;
    ComPtr<ID3D12Device14> m_Device;
    ComPtr<IDXGISwapChain4> m_Swapchain;
    ComPtr<ID3D12CommandQueue> m_GraphicsCommandQueue;

public:
    DX12Renderer(WindowHandle Window, IDXGIAdapter4* GPUAdapter = nullptr);
};

}
