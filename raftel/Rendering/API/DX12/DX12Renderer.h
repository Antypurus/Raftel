#pragma once

#include <Rendering/API/DX12/DX12.h>

namespace raftel::dx12 {

using namespace Microsoft::WRL;

struct DX12Renderer {
private:
    ComPtr<ID3D12Debug6> m_debug_controller;
    ComPtr<IDXGIFactory7> m_dxgi_factory;

public:
    DX12Renderer(HWND window_handle);
};

}
