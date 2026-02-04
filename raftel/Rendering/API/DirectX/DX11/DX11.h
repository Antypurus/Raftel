#pragma once

#include <Rendering/API/DirectX/DXGI/dxgi.h>
#include <Windowing/Window.h>

#include <d3d11_4.h>
#include <string>
#include <vector>
#include <wrl.h>

namespace raftel::dx11 {

using namespace Microsoft::WRL;

struct GPUDevice {
private:
    ComPtr<ID3D11Device5> m_device = nullptr;

public:
    static GPUDevice CreateDevice();

public:
    GPUDevice(ComPtr<ID3D11Device5> device);

    ID3D11Device5* get();
    ID3D11Device5* operator->();

    ComPtr<IDXGISwapChain4> CreateSwapchain(WindowHandle handle);

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
