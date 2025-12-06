#include <cassert>
#include <iostream>

#include "Rendering/API/D3D12/D3D12.h"
#include "Windowing/Window.h"

using namespace raftel;

int main()
{
    WindowingSystem& windowing_system = WindowingSystem::get_instance();
    WindowHandle first_window = windowing_system.create_window("test_window", 1920, 1080);

#ifdef _WIN32
    using namespace Microsoft::WRL;
    ComPtr<ID3D12Debug> debug_controller;
    HRESULT result = D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller));
    if (result != S_OK) {
        std::cerr << "Failed to load D3D12 debug controller" << std::endl;
        exit(-1);
    }
    debug_controller->EnableDebugLayer();
    std::cout << "D3D12 Debug Controller Loaded & Activated" << std::endl;
#endif

    while (windowing_system.has_open_windows()) {
        windowing_system.update();
        auto handles = windowing_system.get_active_window_list();
        for (auto& window : handles) {
            if (!windowing_system.is_window_open(window))
                continue;
        }
    }

    return 0;
}
