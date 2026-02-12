#pragma once

#include <Rendering/API/Windows/DirectX/DXGI/dxgi.h>
#include <Windowing/Window.h>

#include <d3d11.h>
#include <d3d11_4.h>
#include <string>
#include <vector>
#include <wrl.h>

namespace raftel::dx11 {

using namespace Microsoft::WRL;

struct Swapchain {
public:
    ComPtr<IDXGISwapChain4> swapchain = nullptr;
    ComPtr<ID3D11Texture2D> depth_buffer = nullptr;
    ComPtr<ID3D11DepthStencilView> depth_buffer_dsv = nullptr;
    ComPtr<ID3D11RenderTargetView> backbuffer_rtv = nullptr;

public:
    void Present();
};

struct GPUDevice {
public:
    ComPtr<ID3D11DeviceContext> context = nullptr;
    ComPtr<ID3D11Device5> device = nullptr;

public:
    static GPUDevice CreateDevice();

public:
    ID3D11Device5* get();
    ID3D11Device5* operator->();

    Swapchain CreateSwapchain(WindowHandle handle, dxgi::ResourceFormat format = dxgi::ResourceFormat::BGRA8Unorm);

    void Clear(const Swapchain& swapchain);

    void DumpErrorMessages() const;
    std::vector<std::string> GetErrorMessages() const;
};

void init_d3d11(WindowHandle window);

}

#define DX11_ERROR_MESSAGE_PUMP(device)                                                          \
    {                                                                                            \
        const auto CAT(d3d_error_messages, __LINE__) = (device).GetErrorMessages();              \
        for (const auto& CAT(d3d_error_message, __LINE__) : CAT(d3d_error_messages, __LINE__)) { \
            LOG_ERROR("{}", CAT(d3d_error_message, __LINE__));                                   \
        }                                                                                        \
    }
#define DX11_CALL(function_call, device, error_message, ...) WIN_CALL_GUARD(function_call, error_message, DX11_ERROR_MESSAGE_PUMP(device), __VA_ARGS__)
