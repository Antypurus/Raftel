#include "DX12Renderer.h"

#include <iostream>
#include <logger.h>

namespace raftel::dx12 {

DX12Renderer::DX12Renderer()
{
    WIN_CALL(D3D12GetDebugInterface(IID_PPV_ARGS(&this->m_debug_controller)),
        "Failed to load D3D12 Debug Controller");
    m_debug_controller->EnableDebugLayer();
    m_debug_controller->SetEnableGPUBasedValidation(TRUE);
    LOG_INFO("D3D12 Debug Controller Loaded & Activated");

    WIN_CALL(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&this->m_dxgi_factory)),
        "Failed to create DXGI Factory");
    LOG_INFO("DXGI Factory created");

    unsigned int adapter_index = 0;
    IDXGIAdapter1* adapter = nullptr;
    while (this->m_dxgi_factory->EnumAdapters1(adapter_index++, &adapter) == S_OK) {
        IDXGIAdapter4* interface_adapter = nullptr;
        WIN_CALL(adapter->QueryInterface(IID_PPV_ARGS(&interface_adapter)),
            "Failed to move factory interface from version 1 to version 4");

        DXGI_ADAPTER_DESC3 adapter_desc;
        interface_adapter->GetDesc3(&adapter_desc);
        std::wcout << adapter_index - 1 << ":" << adapter_desc.Description << std::endl;

        interface_adapter->Release();
    }
}

}
