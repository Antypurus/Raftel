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

#define DX11_CALL(function_call, device, error_message, ...)                                                    \
    {                                                                                                           \
        HRESULT result##__LINE__ = (function_call);                                                             \
        if (result##__LINE__ != S_OK && result##__LINE__ != S_FALSE) {                                          \
            auto error_message_##__LINE__ = raftel::dxgi::TranslateWindowsErrorCode(result##__LINE__);           \
            LOG_ERROR(error_message, __VA_ARGS__);                                                              \
            LOG_ERROR("{}", std::string_view(error_message_##__LINE__.first, error_message_##__LINE__.second)); \
            LocalFree((LPSTR)error_message_##__LINE__.first);                                                   \
            auto d3d_error_messages = (device).GetErrorMessages();                                              \
            for (const auto& message : d3d_error_messages) {                                                    \
                LOG_ERROR("{}", message);                                                                       \
            }                                                                                                   \
            __debugbreak();                                                                                     \
            exit(-1);                                                                                           \
        }                                                                                                       \
    }
