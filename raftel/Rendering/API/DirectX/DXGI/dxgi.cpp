#include "dxgi.h"

namespace raftel::dxgi {

ComPtr<IDXGIFactory7> GetDXGIFactory()
{
    ComPtr<IDXGIFactory7> factory = nullptr;
#ifndef NDEBUG
    unsigned int flags = DXGI_CREATE_FACTORY_DEBUG;
#else
    unsigned int flags = 0;
#endif
    WIN_CALL(CreateDXGIFactory2(flags, IID_PPV_ARGS(&factory)), "Failed to create temporary factory for device listing");

    return factory;
}

}
