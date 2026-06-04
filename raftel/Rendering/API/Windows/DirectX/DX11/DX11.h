#pragma once

#include <Rendering/API/Windows/DirectX/DXGI/dxgi.h>
#include <Windowing/Window.h>

#include <d3d11.h>
#include <d3d11_4.h>
#include <d3dcommon.h>
#include <dxgi1_5.h>
#include <wrl.h>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace raftel::dx11 {

using namespace Microsoft::WRL;

enum class ShaderType {
    Pixel,
    Vertex,
    Compute,
};

struct SwapchainResources {
    ComPtr<ID3D11Texture2D> DepthBuffer = nullptr;
    ComPtr<ID3D11DepthStencilView> DepthBufferDSV = nullptr;
    ComPtr<ID3D11RenderTargetView> BackbufferRTV = nullptr;
};

struct Swapchain {
public:
    WindowHandle Window;
    ComPtr<IDXGISwapChain4> Swapchain = nullptr;
    SwapchainResources Resources;

    bool NeedsResize = false;
    std::uint32_t NewWidth = 0;
    std::uint32_t NewHeight = 0;

public:
    void Present();
    void RegisterResize(std::uint32_t NewWidth, std::uint32_t NewHeight);
};

template <typename ShaderT>
struct Shader {
public:
    ComPtr<ShaderT> ShaderProgram = nullptr;
    ComPtr<ID3DBlob> Bytecode = nullptr;
};
using VertexShader = Shader<ID3D11VertexShader>;
using PixelShader = Shader<ID3D11PixelShader>;
using ComputeShader = Shader<ID3D11ComputeShader>;

struct VertexBuffer {
public:
    ComPtr<ID3D11Buffer> Buffer = nullptr;
    ComPtr<ID3D11InputLayout> VertexLayout = nullptr;
};

struct IndexBuffer {
    ComPtr<ID3D11Buffer> Buffer = nullptr;
};

struct GPUDevice {
public:
    ComPtr<ID3D11DeviceContext> Context = nullptr;
    ComPtr<ID3D11Device5> Device = nullptr;

public:
    static GPUDevice CreateDevice();

public:
    ID3D11Device5* Get();
    ID3D11Device5* operator->();

    Swapchain CreateSwapchain(WindowHandle Handle, dxgi::ResourceFormat Format = dxgi::ResourceFormat::BGRA8Unorm);
    SwapchainResources CreateSwapchainResources(ComPtr<IDXGISwapChain4> Swapchain, Resolution Size);
    void Bind(const Swapchain& Swapchain);

    std::optional<ComPtr<ID3DBlob>> CompileShader(std::wstring_view Path, std::string_view Entrypoint, ShaderType Type);
    std::optional<VertexShader> CompileVertexShader(std::wstring_view Path, std::string_view Entrypoint = "VSMain");
    std::optional<PixelShader> CompilePixelShader(std::wstring_view Path, std::string_view Entrypoint = "PSMain");
    std::optional<ComputeShader> CompileComputeShader(std::wstring_view Path, std::string_view Entrypoint = "CSMain");

    VertexBuffer CreateVertexBuffer(const std::vector<float>& Vertices, Shader<ID3D11VertexShader> VertexShader);
    IndexBuffer CreateIndexBuffer(const std::vector<std::uint32_t>& Indices);

    template <typename T>
    void Bind(const Shader<T>& Shader);
    void Bind(const VertexBuffer& VertexBuffer);
    void Bind(const IndexBuffer& IndexBuffer);

    void DrawTriangles(std::uint32_t Count);
    void Clear(Swapchain& Swapchain);

    void DumpErrorMessages() const;
    std::vector<std::string> GetErrorMessages() const;
};

template <typename T>
void GPUDevice::Bind(const Shader<T>& p_Shader)
{
    if constexpr (std::is_same_v<T, ID3D11VertexShader>) {
        this->Context->VSSetShader(p_Shader.ShaderProgram.Get(), nullptr, 0);
    } else if constexpr (std::is_same_v<T, ID3D11PixelShader>) {
        this->Context->PSSetShader(p_Shader.ShaderProgram.Get(), nullptr, 0);
    } else if constexpr (std::is_same_v<T, ID3D11ComputeShader>) {
        this->Context->CSSetShader(p_Shader.ShaderProgram.Get(), nullptr, 0);
    } else {
        static_assert(false, "Non-Valid Shader Type");
    }
}

}

#define DX11_ERROR_MESSAGE_PUMP(p_Device, p_PostAction)                                          \
    {                                                                                            \
        const auto CAT(d3d_error_messages, __LINE__) = (p_Device).GetErrorMessages();            \
        for (const auto& CAT(d3d_error_message, __LINE__) : CAT(d3d_error_messages, __LINE__)) { \
            LOG_ERROR("{}", CAT(d3d_error_message, __LINE__));                                   \
            p_PostAction;                                                                        \
        }                                                                                        \
    }
#define DX11_CALL(p_FunctionCall, p_Device, p_ErrorMessage, ...) WIN_CALL_GUARD(p_FunctionCall, p_ErrorMessage, DX11_ERROR_MESSAGE_PUMP(p_Device, { }), __VA_ARGS__)
#define DX11_CALL_WITH_POST(p_FunctionCall, p_Device, p_PostAction, p_ErrorMessage, ...) WIN_CALL_GUARD(p_FunctionCall, p_ErrorMessage, DX11_ERROR_MESSAGE_PUMP(p_Device, p_PostAction), __VA_ARGS__)
