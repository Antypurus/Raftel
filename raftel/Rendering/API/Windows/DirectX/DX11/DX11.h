#pragma once

#include <Rendering/API/Windows/DirectX/DXGI/dxgi.h>
#include <Windowing/Window.h>

#include <atomic>
#include <cstdint>
#include <d3d11.h>
#include <d3d11_4.h>
#include <d3dcommon.h>
#include <dxgi1_5.h>
#include <expected>
#include <optional>
#include <string>
#include <vector>
#include <wrl.h>

namespace raftel::dx11 {

using namespace Microsoft::WRL;

enum class ShaderType {
    Pixel,
    Vertex,
    Compute,
};

struct SwapchainResources {
    ComPtr<ID3D11Texture2D> depth_buffer = nullptr;
    ComPtr<ID3D11DepthStencilView> depth_buffer_dsv = nullptr;
    ComPtr<ID3D11RenderTargetView> backbuffer_rtv = nullptr;
};

struct Swapchain {
public:
    WindowHandle window;
    ComPtr<IDXGISwapChain4> swapchain = nullptr;
    SwapchainResources resources;

    bool needs_resize = false;
    std::uint32_t new_width = 0;
    std::uint32_t new_height = 0;

public:
    void Present();
    void RegisterResize(std::uint32_t new_width, std::uint32_t new_height);
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
    SwapchainResources CreateSwapchainResources(ComPtr<IDXGISwapChain4> swapchain, Resolution size);

    std::optional<ComPtr<ID3DBlob>> CompileShader(std::wstring_view path, std::string_view entrypoint, ShaderType type);
    std::optional<ComPtr<ID3D11VertexShader>> CompileVertexShader(std::wstring_view path, std::string_view entrypoint = "VSMain");
    std::optional<ComPtr<ID3D11PixelShader>> CompilePixelShader(std::wstring_view path, std::string_view entrypoint = "PSMain");
    std::optional<ComPtr<ID3D11ComputeShader>> CompileComputeShader(std::wstring_view path, std::string_view entrypoint = "CSMain");

    ComPtr<ID3D11Buffer> CreateVertexBuffer(const std::vector<float>& vertices);

    void Clear(Swapchain& swapchain);

    void DumpErrorMessages() const;
    std::vector<std::string> GetErrorMessages() const;
};

void init_d3d11(WindowHandle window);

}

#define DX11_ERROR_MESSAGE_PUMP(device, post_action)                                             \
    {                                                                                            \
        const auto CAT(d3d_error_messages, __LINE__) = (device).GetErrorMessages();              \
        for (const auto& CAT(d3d_error_message, __LINE__) : CAT(d3d_error_messages, __LINE__)) { \
            LOG_ERROR("{}", CAT(d3d_error_message, __LINE__));                                   \
            post_action;                                                                         \
        }                                                                                        \
    }
#define DX11_CALL(function_call, device, error_message, ...) WIN_CALL_GUARD(function_call, error_message, DX11_ERROR_MESSAGE_PUMP(device, {}), __VA_ARGS__)
#define DX11_CALL_WITH_POST(function_call, device, post_action, error_message, ...) WIN_CALL_GUARD(function_call, error_message, DX11_ERROR_MESSAGE_PUMP(device, post_action), __VA_ARGS__)
