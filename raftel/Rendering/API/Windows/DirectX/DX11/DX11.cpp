#include "DX11.h"

#include <Rendering/API/Windows/DirectX/DX12/DX12Renderer.h>
#include <Rendering/API/Windows/DirectX/DXGI/dxgi.h>
#include <Windowing/Window.h>
#include <logger.h>

#include <assert.h>
#include <cstdint>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <iostream>
#include <optional>

namespace raftel::dx11 {

void Swapchain::Present()
{
    DXGI_CALL(this->swapchain->Present(0, DXGI_PRESENT_ALLOW_TEARING), "Swapchain present failed");
}

void Swapchain::RegisterResize(std::uint32_t width, std::uint32_t height)
{
    this->new_width = width;
    this->new_height = height;
    this->needs_resize = true;
}

GPUDevice GPUDevice::CreateDevice()
{
    std::vector<dx12::AdapterInfo> adaptors = dx12::GetDeviceList();
    assert(adaptors.size() > 0);

    IDXGIAdapter* p_adapter = adaptors[0].adapter.Get();
    std::wcout << "Selected Adapter: " << adaptors[0].name << std::endl;

    D3D_FEATURE_LEVEL features_levels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_11_1
    };
    D3D_FEATURE_LEVEL selected_feature_level;

    ComPtr<ID3D11Device> device = nullptr;
    ComPtr<ID3D11DeviceContext> context = nullptr;
    WIN_CALL(D3D11CreateDevice(
                 p_adapter,
                 D3D_DRIVER_TYPE_UNKNOWN,
                 nullptr,
                 D3D11_CREATE_DEVICE_DEBUG,
                 features_levels,
                 _countof(features_levels),
                 D3D11_SDK_VERSION,
                 device.GetAddressOf(),
                 &selected_feature_level,
                 context.GetAddressOf()),
        "Failed to create D3D11 Device");

    ComPtr<ID3D11Device5> final_dev = nullptr;
    WIN_CALL(device->QueryInterface(IID_PPV_ARGS(&final_dev)), "Failed to upcast D3D11 device to version 5");
    return GPUDevice {
        .context = context,
        .device = final_dev,
    };
}

ID3D11Device5* GPUDevice::operator->()
{
    return this->device.Get();
}

ID3D11Device5* GPUDevice::get()
{
    return this->device.Get();
}

std::vector<std::string> GPUDevice::GetErrorMessages() const
{
    std::vector<std::string> error_messages;

    ComPtr<ID3D11InfoQueue> info_queue = nullptr;
    WIN_CALL(this->device->QueryInterface(IID_PPV_ARGS(&info_queue)), "Failed to obtain D3D11 Info-Queue");
    const std::uint64_t message_count = info_queue->GetNumStoredMessages();
    for (size_t i = 0; i < message_count; ++i) {
        size_t message_length = 0;
        WIN_CALL(info_queue->GetMessage(i, nullptr, &message_length), "Failed to get error message length");

        D3D11_MESSAGE* message = (D3D11_MESSAGE*)malloc(message_length);
        WIN_CALL(info_queue->GetMessage(i, message, &message_length), "Failed to get error message");

        error_messages.emplace_back(message->pDescription, message->DescriptionByteLength);
        free(message);
    }
    info_queue->ClearStoredMessages();
    return error_messages;
}

void GPUDevice::DumpErrorMessages() const
{
    std::vector<std::string> error_messages = this->GetErrorMessages();
    for (const auto& message : error_messages) {
        LOG_ERROR("{}", message);
    }
}

SwapchainResources GPUDevice::CreateSwapchainResources(ComPtr<IDXGISwapChain4> swapchain, Resolution size)
{
    // create the render target views for the swapchain
    ComPtr<ID3D11Texture2D> backbuffer_resouce = nullptr;
    ComPtr<ID3D11RenderTargetView> backbuffer_rtv = nullptr;
    DXGI_CALL(swapchain->GetBuffer(0, IID_PPV_ARGS(&backbuffer_resouce)), "Failed to get swapchain backbuffer");
    DX11_CALL(this->device->CreateRenderTargetView(backbuffer_resouce.Get(), nullptr, backbuffer_rtv.GetAddressOf()), *this, "Failed to create RTV for Backbuffer");
    LOG_SUCCESS("Swapchain backbuffer and render target view created");

    // create the depth stenvil view for the swapchain
    ComPtr<ID3D11Texture2D> depth_stentil_buffer = nullptr;
    ComPtr<ID3D11DepthStencilView> depth_stencil_view = nullptr;
    const D3D11_TEXTURE2D_DESC depth_stentil_buffer_desc = {
        .Width = size.width,
        .Height = size.height,
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
    DX11_CALL(this->device->CreateTexture2D(&depth_stentil_buffer_desc, nullptr, depth_stentil_buffer.GetAddressOf()), *this, "Failed to create depth stencil buffer for swapchain");
    DX11_CALL(this->device->CreateDepthStencilView(depth_stentil_buffer.Get(), nullptr, depth_stencil_view.GetAddressOf()), *this, "Failed to create depth stencil view for swapchain");
    LOG_SUCCESS("Swapchain depth stencil buffer & view created");

    return SwapchainResources {
        .depth_buffer = depth_stentil_buffer,
        .depth_buffer_dsv = depth_stencil_view,
        .backbuffer_rtv = backbuffer_rtv,
    };
}

Swapchain GPUDevice::CreateSwapchain(WindowHandle window, dxgi::ResourceFormat format)
{
    WindowingSystem& window_system = WindowingSystem::get_instance();
    Resolution window_resolution = window_system.get_window_resolution(window);

    auto& factory = dxgi::DXGIFactory::GetFactory();
    auto swapchain = factory.CreateSwapchain(this->device.Get(), window, dxgi::SwapchainParams { .format = format });
    LOG_SUCCESS("Swapchain Created");

    return Swapchain {
        .window = window,
        .swapchain = swapchain,
        .resources = this->CreateSwapchainResources(swapchain, window_resolution),
    };
}

ComPtr<ID3D11Buffer> GPUDevice::CreateVertexBuffer(const std::vector<float>& vertices)
{
    const D3D11_INPUT_ELEMENT_DESC vertex_desc[] = {
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
    ComPtr<ID3D11InputLayout> vertex_layout = nullptr;
    DX11_CALL(this->device->CreateInputLayout(vertex_desc, _countof(vertex_desc), nullptr, 0, vertex_layout.GetAddressOf()), *this, "Failed to create vertex buffer");

    const D3D11_BUFFER_DESC buffer_desc = {
        .ByteWidth = (std::uint32_t)(vertices.size() * sizeof(float)),
        .Usage = D3D11_USAGE_IMMUTABLE,
        .BindFlags = D3D11_BIND_VERTEX_BUFFER,
        .CPUAccessFlags = 0,
        .MiscFlags = 0,
        .StructureByteStride = 0,
    };
    const D3D11_SUBRESOURCE_DATA resource_desc = {
        .pSysMem = vertices.data(),
        .SysMemPitch = 0,
        .SysMemSlicePitch = 0,
    };

    ComPtr<ID3D11Buffer> vertex_buffer = nullptr;
    DX11_CALL(this->device->CreateBuffer(&buffer_desc, &resource_desc, vertex_buffer.GetAddressOf()), *this, "Failed to create vertex buffer");
    return vertex_buffer;
}

void GPUDevice::Clear(Swapchain& swapchain)
{
    const float clear_value[4] = { 1.0, 0.0, 0.0, 1.0 };
    this->context->ClearDepthStencilView(swapchain.resources.depth_buffer_dsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0, 0);
    this->context->ClearRenderTargetView(swapchain.resources.backbuffer_rtv.Get(), clear_value);

    if (swapchain.needs_resize) {
        // unbind swapchain references
        this->context->OMSetRenderTargets(0, nullptr, nullptr);
        this->context->OMSetDepthStencilState(nullptr, 0);
        this->context->Flush();

        swapchain.resources.depth_buffer_dsv.Reset();
        swapchain.resources.depth_buffer.Reset();
        swapchain.resources.backbuffer_rtv.Reset();

        DXGI_CALL(swapchain.swapchain->ResizeBuffers(2, swapchain.new_width, swapchain.new_height, (DXGI_FORMAT)dxgi::ResourceFormat::BGRA8Unorm, DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING), "Failed to resize swapchain");
        swapchain.resources = this->CreateSwapchainResources(swapchain.swapchain, {
                                                                                      .width = swapchain.new_width,
                                                                                      .height = swapchain.new_height,
                                                                                  });

        swapchain.needs_resize = false;
    }
}

std::optional<ComPtr<ID3DBlob>> GPUDevice::CompileShader(std::wstring_view path, std::string_view entrypoint, ShaderType type)
{
    const std::unordered_map<ShaderType, const char*> compilation_profiles = {
        { ShaderType::Vertex, "vs_5_0" },
        { ShaderType::Pixel, "ps_5_0" },
        { ShaderType::Compute, "cs_5_0" },
    };

    ComPtr<ID3DBlob> shader_blob = nullptr;
    ComPtr<ID3DBlob> error_blob = nullptr;
    HRESULT result = D3DCompileFromFile(
        path.data(),
        nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entrypoint.data(), compilation_profiles.at(type),
        D3DCOMPILE_ENABLE_STRICTNESS, 0,
        &shader_blob, &error_blob);
    if (FAILED(result)) {
        LOG_ERROR("Shader Blob Compilation Failed");
        return {};
    }
    LOG_SUCCESS("Shader blob compilation finished for: {}", entrypoint);

    return std::make_optional(shader_blob);
}

std::optional<ComPtr<ID3D11VertexShader>> GPUDevice::CompileVertexShader(std::wstring_view path, std::string_view entrypoint)
{
    std::optional<ComPtr<ID3DBlob>> compilation_result = this->CompileShader(path, entrypoint, ShaderType::Vertex);
    if (!compilation_result.has_value()) {
        return {};
    }
    ComPtr<ID3DBlob> shader_blob = std::move(compilation_result.value());

    ComPtr<ID3D11VertexShader> vertex_shader = nullptr;
    DX11_CALL_WITH_POST(this->device->CreateVertexShader(
                            shader_blob->GetBufferPointer(),
                            shader_blob->GetBufferSize(),
                            nullptr, vertex_shader.GetAddressOf()),
        *this,
        return {};
        , "Failed to create vertex shader object");

    LOG_SUCCESS("Vertex Shader Object Created");
    return std::make_optional(vertex_shader);
}

std::optional<ComPtr<ID3D11PixelShader>> GPUDevice::CompilePixelShader(std::wstring_view path, std::string_view entrypoint)
{
    std::optional<ComPtr<ID3DBlob>> compilation_result = this->CompileShader(path, entrypoint, ShaderType::Pixel);
    if (!compilation_result.has_value()) {
        return {};
    }
    ComPtr<ID3DBlob> shader_blob = std::move(compilation_result.value());

    ComPtr<ID3D11PixelShader> pixel_shader = nullptr;
    DX11_CALL_WITH_POST(this->device->CreatePixelShader(
                            shader_blob->GetBufferPointer(),
                            shader_blob->GetBufferSize(),
                            nullptr, pixel_shader.GetAddressOf()),
        *this,
        return {};
        , "Failed to create pixel shader object");

    LOG_SUCCESS("Pixel Shader Object Created");
    return std::make_optional(pixel_shader);
}

std::optional<ComPtr<ID3D11ComputeShader>> GPUDevice::CompileComputeShader(std::wstring_view path, std::string_view entrypoint)
{
    std::optional<ComPtr<ID3DBlob>> compilation_result = this->CompileShader(path, entrypoint, ShaderType::Compute);
    if (!compilation_result.has_value()) {
        return {};
    }
    ComPtr<ID3DBlob> shader_blob = std::move(compilation_result.value());

    ComPtr<ID3D11ComputeShader> compute_shader = nullptr;
    DX11_CALL_WITH_POST(this->device->CreateComputeShader(
                            shader_blob->GetBufferPointer(),
                            shader_blob->GetBufferSize(),
                            nullptr, compute_shader.GetAddressOf()),
        *this,
        return {};
        , "Failed to create compute shader object");

    LOG_SUCCESS("Compute Shader Object Created");
    return std::make_optional(compute_shader);
}

void init_d3d11(WindowHandle window)
{
    auto device = GPUDevice::CreateDevice();
    auto swapchain = device.CreateSwapchain(window);
    device.Clear(swapchain);
    device.DumpErrorMessages();
}
}
