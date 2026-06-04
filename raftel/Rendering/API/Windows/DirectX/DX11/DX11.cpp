#include "DX11.h"

#include <Rendering/API/Windows/DirectX/DX12/DX12Renderer.h>
#include <Rendering/API/Windows/DirectX/DXGI/dxgi.h>
#include <Windowing/Window.h>

#include <d3d11.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>
#include <dxgi.h>

#include <assert.h>
#include <iostream>
#include <optional>

namespace raftel::dx11 {

void Swapchain::Present()
{
    DXGI_CALL(this->Swapchain->Present(0, DXGI_PRESENT_ALLOW_TEARING), "Swapchain present failed");
}

void Swapchain::RegisterResize(std::uint32_t p_Width, std::uint32_t p_Height)
{
    this->NewWidth = p_Width;
    this->NewHeight = p_Height;
    this->NeedsResize = true;
}

GPUDevice GPUDevice::CreateDevice()
{
    std::vector<dx12::AdapterInfo> adaptors = dx12::GetDeviceList();
    assert(adaptors.size() > 0);

    IDXGIAdapter* adapter = adaptors[0].Adapter.Get();
    std::wcout << "Selected Adapter: " << adaptors[0].Name << std::endl;

    D3D_FEATURE_LEVEL featuresLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_11_1
    };
    D3D_FEATURE_LEVEL selectedFeatureLevel;

    ComPtr<ID3D11Device> device = nullptr;
    ComPtr<ID3D11DeviceContext> context = nullptr;
    WIN_CALL(D3D11CreateDevice(
                 adapter,
                 D3D_DRIVER_TYPE_UNKNOWN,
                 nullptr,
                 D3D11_CREATE_DEVICE_DEBUG,
                 featuresLevels,
                 _countof(featuresLevels),
                 D3D11_SDK_VERSION,
                 device.GetAddressOf(),
                 &selectedFeatureLevel,
                 context.GetAddressOf()),
        "Failed to create D3D11 Device");

    ComPtr<ID3D11Device5> finalDevice = nullptr;
    WIN_CALL(device->QueryInterface(IID_PPV_ARGS(&finalDevice)), "Failed to upcast D3D11 device to version 5");
    return GPUDevice {
        .Context = context,
        .Device = finalDevice,
    };
}

ID3D11Device5* GPUDevice::operator->()
{
    return this->Device.Get();
}

ID3D11Device5* GPUDevice::Get()
{
    return this->Device.Get();
}

std::vector<std::string> GPUDevice::GetErrorMessages() const
{
    std::vector<std::string> errorMessages;

    ComPtr<ID3D11InfoQueue> infoQueue = nullptr;
    WIN_CALL(this->Device->QueryInterface(IID_PPV_ARGS(&infoQueue)), "Failed to obtain D3D11 Info-Queue");
    const std::uint64_t messageCount = infoQueue->GetNumStoredMessages();
    for (size_t i = 0; i < messageCount; ++i) {
        size_t messageLength = 0;
        WIN_CALL(infoQueue->GetMessage(i, nullptr, &messageLength), "Failed to get error message length");

        D3D11_MESSAGE* message = (D3D11_MESSAGE*)malloc(messageLength);
        WIN_CALL(infoQueue->GetMessage(i, message, &messageLength), "Failed to get error message");

        errorMessages.emplace_back(message->pDescription, message->DescriptionByteLength);
        free(message);
    }
    infoQueue->ClearStoredMessages();
    return errorMessages;
}

void GPUDevice::DumpErrorMessages() const
{
    std::vector<std::string> errorMessages = this->GetErrorMessages();
    for (const auto& message : errorMessages) {
        LOG_ERROR("{}", message);
    }
}

SwapchainResources GPUDevice::CreateSwapchainResources(ComPtr<IDXGISwapChain4> p_Swapchain, Resolution p_Size)
{
    // create the render target views for the swapchain
    ComPtr<ID3D11Texture2D> backbufferResource = nullptr;
    ComPtr<ID3D11RenderTargetView> backbufferRTV = nullptr;
    DXGI_CALL(p_Swapchain->GetBuffer(0, IID_PPV_ARGS(&backbufferResource)), "Failed to get swapchain backbuffer");
    DX11_CALL(this->Device->CreateRenderTargetView(backbufferResource.Get(), nullptr, backbufferRTV.GetAddressOf()), *this, "Failed to create RTV for Backbuffer");
    LOG_SUCCESS("Swapchain backbuffer and render target view created");

    // create the depth stenvil view for the swapchain
    ComPtr<ID3D11Texture2D> depthStencilBuffer = nullptr;
    ComPtr<ID3D11DepthStencilView> depthStencilView = nullptr;
    const D3D11_TEXTURE2D_DESC depthStencilBufferDesc = {
        .Width = p_Size.Width,
        .Height = p_Size.Height,
        .MipLevels = 1,
        .ArraySize = 1,
        .Format = (DXGI_FORMAT)dxgi::ResourceFormat::D24UnormS8Uint,
        .SampleDesc = {
            .Count = 1,
            .Quality = 0,
        },
        .Usage = D3D11_USAGE_DEFAULT,
        .BindFlags = D3D11_BIND_DEPTH_STENCIL,
        .CPUAccessFlags = 0,
        .MiscFlags = 0,
    };
    DX11_CALL(this->Device->CreateTexture2D(&depthStencilBufferDesc, nullptr, depthStencilBuffer.GetAddressOf()), *this, "Failed to create depth stencil buffer for swapchain");
    DX11_CALL(this->Device->CreateDepthStencilView(depthStencilBuffer.Get(), nullptr, depthStencilView.GetAddressOf()), *this, "Failed to create depth stencil view for swapchain");
    LOG_SUCCESS("Swapchain depth stencil buffer & view created");

    return SwapchainResources {
        .DepthBuffer = depthStencilBuffer,
        .DepthBufferDSV = depthStencilView,
        .BackbufferRTV = backbufferRTV,
    };
}

Swapchain GPUDevice::CreateSwapchain(WindowHandle p_Window, dxgi::ResourceFormat p_Format)
{
    WindowingSystem& windowSystem = WindowingSystem::GetInstance();
    Resolution windowResolution = windowSystem.GetWindowResolution(p_Window);

    auto& factory = dxgi::DXGIFactory::GetFactory();
    auto swapchain = factory.CreateSwapchain(this->Device.Get(), p_Window, dxgi::SwapchainParams { .Format = p_Format });
    LOG_SUCCESS("Swapchain Created");

    return Swapchain {
        .Window = p_Window,
        .Swapchain = swapchain,
        .Resources = this->CreateSwapchainResources(swapchain, windowResolution),
        .NewWidth = windowResolution.Width,
        .NewHeight = windowResolution.Height,
    };
}

VertexBuffer GPUDevice::CreateVertexBuffer(const std::vector<float>& p_Vertices, Shader<ID3D11VertexShader> p_VertexShader)
{
    const D3D11_INPUT_ELEMENT_DESC vertexDesc[] = {
        D3D11_INPUT_ELEMENT_DESC {
            .SemanticName = "POSITION",
            .SemanticIndex = 0,
            .Format = DXGI_FORMAT_R32G32B32_FLOAT,
            .InputSlot = 0,
            .AlignedByteOffset = 0,
            .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
            .InstanceDataStepRate = 0,
        },
    };
    ComPtr<ID3D11InputLayout> vertexLayout = nullptr;
    DX11_CALL(this->Device->CreateInputLayout(
                  vertexDesc, _countof(vertexDesc),
                  p_VertexShader.Bytecode->GetBufferPointer(), p_VertexShader.Bytecode->GetBufferSize(),
                  vertexLayout.GetAddressOf()),
        *this, "Failed to create vertex buffer");

    const D3D11_BUFFER_DESC bufferDesc = {
        .ByteWidth = (std::uint32_t)(p_Vertices.size() * sizeof(float)),
        .Usage = D3D11_USAGE_IMMUTABLE,
        .BindFlags = D3D11_BIND_VERTEX_BUFFER,
        .CPUAccessFlags = 0,
        .MiscFlags = 0,
        .StructureByteStride = 0,
    };
    const D3D11_SUBRESOURCE_DATA resourceDesc = {
        .pSysMem = p_Vertices.data(),
        .SysMemPitch = 0,
        .SysMemSlicePitch = 0,
    };
    ComPtr<ID3D11Buffer> vertexBuffer = nullptr;
    DX11_CALL(this->Device->CreateBuffer(&bufferDesc, &resourceDesc, vertexBuffer.GetAddressOf()), *this, "Failed to create vertex buffer");

    return VertexBuffer {
        .Buffer = vertexBuffer,
        .VertexLayout = vertexLayout,
    };
}

IndexBuffer GPUDevice::CreateIndexBuffer(const std::vector<std::uint32_t>& p_Indices)
{
    const D3D11_BUFFER_DESC bufferDesc = {
        .ByteWidth = (std::uint32_t)(p_Indices.size() * sizeof(std::uint32_t)),
        .Usage = D3D11_USAGE_IMMUTABLE,
        .BindFlags = D3D11_BIND_INDEX_BUFFER,
        .CPUAccessFlags = 0,
        .MiscFlags = 0,
        .StructureByteStride = 0,
    };
    const D3D11_SUBRESOURCE_DATA resourceDesc = {
        .pSysMem = p_Indices.data(),
        .SysMemPitch = 0,
        .SysMemSlicePitch = 0,
    };

    ComPtr<ID3D11Buffer> indexBuffer = nullptr;
    DX11_CALL(this->Device->CreateBuffer(&bufferDesc, &resourceDesc, indexBuffer.GetAddressOf()), *this, "Failed to create index buffer");

    return {
        .Buffer = indexBuffer,
    };
}

void GPUDevice::Bind(const VertexBuffer& p_VertexBuffer)
{
    std::uint32_t stride = 3 * sizeof(float);
    std::uint32_t offsett = 0;
    this->Context->IASetVertexBuffers(0, 1, p_VertexBuffer.Buffer.GetAddressOf(), &stride, &offsett);
    this->Context->IASetInputLayout(p_VertexBuffer.VertexLayout.Get());
}

void GPUDevice::Bind(const IndexBuffer& p_IndexBuffer)
{
    this->Context->IASetIndexBuffer(p_IndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
}

void GPUDevice::Bind(const Swapchain& p_Swapchain)
{
    const D3D11_VIEWPORT viewport = {
        .TopLeftX = 0.0f,
        .TopLeftY = 0.0f,
        .Width = static_cast<float>(p_Swapchain.NewWidth),
        .Height = static_cast<float>(p_Swapchain.NewHeight),
        .MinDepth = 0.0f,
        .MaxDepth = 1.0f,
    };
    this->Context->RSSetViewports(1, &viewport);
    this->Context->OMSetRenderTargets(1, p_Swapchain.Resources.BackbufferRTV.GetAddressOf(), p_Swapchain.Resources.DepthBufferDSV.Get());
}

void GPUDevice::DrawTriangles(std::uint32_t p_Count)
{
    this->Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    this->Context->DrawIndexed(p_Count, 0, 0);
}

void GPUDevice::Clear(Swapchain& p_Swapchain)
{
    const float clearValue[4] = { 0.0, 0.0, 0.0, 0.0 };
    this->Context->ClearDepthStencilView(p_Swapchain.Resources.DepthBufferDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0);
    this->Context->ClearRenderTargetView(p_Swapchain.Resources.BackbufferRTV.Get(), clearValue);

    if (p_Swapchain.NeedsResize) {
        // unbind swapchain references
        this->Context->OMSetRenderTargets(0, nullptr, nullptr);
        this->Context->OMSetDepthStencilState(nullptr, 0);
        this->Context->Flush();

        p_Swapchain.Resources.DepthBufferDSV.Reset();
        p_Swapchain.Resources.DepthBuffer.Reset();
        p_Swapchain.Resources.BackbufferRTV.Reset();

        DXGI_CALL(p_Swapchain.Swapchain->ResizeBuffers(2, p_Swapchain.NewWidth, p_Swapchain.NewHeight, (DXGI_FORMAT)dxgi::ResourceFormat::BGRA8Unorm, DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING), "Failed to resize swapchain");
        p_Swapchain.Resources = this->CreateSwapchainResources(p_Swapchain.Swapchain, {
                                                                                          .Width = p_Swapchain.NewWidth,
                                                                                          .Height = p_Swapchain.NewHeight,
                                                                                      });

        p_Swapchain.NeedsResize = false;
    }
}

std::optional<ComPtr<ID3DBlob>> GPUDevice::CompileShader(std::wstring_view p_Path, std::string_view p_Entrypoint, ShaderType p_Type)
{
    const std::unordered_map<ShaderType, const char*> compilationProfiles = {
        { ShaderType::Vertex, "vs_5_0" },
        { ShaderType::Pixel, "ps_5_0" },
        { ShaderType::Compute, "cs_5_0" },
    };

    ComPtr<ID3DBlob> shaderBlob = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT result = D3DCompileFromFile(
        p_Path.data(),
        nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        p_Entrypoint.data(), compilationProfiles.at(p_Type),
        D3DCOMPILE_ENABLE_STRICTNESS, 0,
        &shaderBlob, &errorBlob);
    if (FAILED(result)) {
        const char* errorMessage = (const char*)errorBlob->GetBufferPointer();
        LOG_ERROR("Shader Blob Compilation Failed: {}", errorMessage);
        return { };
    }

    return std::make_optional(shaderBlob);
}

std::optional<VertexShader> GPUDevice::CompileVertexShader(std::wstring_view p_Path, std::string_view p_Entrypoint)
{
    std::optional<ComPtr<ID3DBlob>> compilationResult = this->CompileShader(p_Path, p_Entrypoint, ShaderType::Vertex);
    if (!compilationResult.has_value()) {
        return { };
    }
    ComPtr<ID3DBlob> shaderBlob = std::move(compilationResult.value());

    ComPtr<ID3D11VertexShader> vertexShader = nullptr;
    DX11_CALL_WITH_POST(this->Device->CreateVertexShader(
                            shaderBlob->GetBufferPointer(),
                            shaderBlob->GetBufferSize(),
                            nullptr, vertexShader.GetAddressOf()),
        *this,
        return { };
        , "Failed to create vertex shader object");

    LOG_SUCCESS("Vertex Shader Object Created");
    return VertexShader {
        .ShaderProgram = vertexShader,
        .Bytecode = shaderBlob,
    };
}

std::optional<PixelShader> GPUDevice::CompilePixelShader(std::wstring_view p_Path, std::string_view p_Entrypoint)
{
    std::optional<ComPtr<ID3DBlob>> compilationResult = this->CompileShader(p_Path, p_Entrypoint, ShaderType::Pixel);
    if (!compilationResult.has_value()) {
        return { };
    }
    ComPtr<ID3DBlob> shaderBlob = std::move(compilationResult.value());

    ComPtr<ID3D11PixelShader> pixelShader = nullptr;
    DX11_CALL_WITH_POST(this->Device->CreatePixelShader(
                            shaderBlob->GetBufferPointer(),
                            shaderBlob->GetBufferSize(),
                            nullptr, pixelShader.GetAddressOf()),
        *this,
        return { };
        , "Failed to create pixel shader object");

    LOG_SUCCESS("Pixel Shader Object Created");
    return PixelShader {
        .ShaderProgram = pixelShader,
        .Bytecode = shaderBlob,
    };
}

std::optional<ComputeShader> GPUDevice::CompileComputeShader(std::wstring_view p_Path, std::string_view p_Entrypoint)
{
    std::optional<ComPtr<ID3DBlob>> compilationResult = this->CompileShader(p_Path, p_Entrypoint, ShaderType::Compute);
    if (!compilationResult.has_value()) {
        return { };
    }
    ComPtr<ID3DBlob> shaderBlob = std::move(compilationResult.value());

    ComPtr<ID3D11ComputeShader> computeShader = nullptr;
    DX11_CALL_WITH_POST(this->Device->CreateComputeShader(
                            shaderBlob->GetBufferPointer(),
                            shaderBlob->GetBufferSize(),
                            nullptr, computeShader.GetAddressOf()),
        *this,
        return { };
        , "Failed to create compute shader object");

    LOG_SUCCESS("Compute Shader Object Created");
    return ComputeShader {
        .ShaderProgram = computeShader,
        .Bytecode = shaderBlob,
    };
}

}
