#include "DX11.h"

#include "Rendering/API/DX12/DX12Renderer.h"

#include <assert.h>
#include <cstdint>
#include <iostream>

#ifdef _WIN32
namespace raftel::dx11 {

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
    return { final_dev };
}

GPUDevice::GPUDevice(ComPtr<ID3D11Device5> device)
    : m_device(device)
{
}

ID3D11Device5* GPUDevice::operator->()
{
    return this->m_device.Get();
}

void GPUDevice::DumpErrorMessages() const
{
    ComPtr<ID3D11InfoQueue> info_queue = nullptr;
    WIN_CALL(this->m_device->QueryInterface(IID_PPV_ARGS(&info_queue)), "Failed to obtain D3D11 Info-Queue");

    const std::uint64_t message_count = info_queue->GetNumStoredMessages();
    for (size_t i = 0; i < message_count; ++i) {
        size_t message_length = 0;
        WIN_CALL(info_queue->GetMessage(i, nullptr, &message_length), "Failed to get error message length");

        D3D11_MESSAGE* message = (D3D11_MESSAGE*)malloc(message_length);
        WIN_CALL(info_queue->GetMessage(i, message, &message_length), "Failed to get error message");

        LOG_ERROR("{}", message->pDescription);
    }
    info_queue->ClearStoredMessages();
}

void init_d3d11()
{
    auto device = GPUDevice::CreateDevice();

    device->CreateBuffer(nullptr, nullptr, nullptr);
    device.DumpErrorMessages();
}

}
#endif
