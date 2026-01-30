#pragma once

#include <d3d11_4.h>
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

    ID3D11Device5* operator->();

    void DumpErrorMessages() const;
};

void init_d3d11();

}
