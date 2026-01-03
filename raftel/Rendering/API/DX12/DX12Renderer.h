#pragma once

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

struct DX12Renderer {
private:
    ComPtr<ID3D12Debug6> m_debug_controller;
    ComPtr<IDXGIFactory7> m_dxgi_factory;

public:
    DX12Renderer(HWND window_handle);

    static std::vector<AdapterInfo> GetDeviceList();
};

}
