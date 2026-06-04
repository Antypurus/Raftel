#include "dxgi.h"

#include <Windowing/Window.h>

#include <combaseapi.h>
#include <dxgidebug.h>
#include <mutex>
#include <unknwnbase.h>

namespace raftel::dxgi {

std::pair<const char*, size_t> TranslateWindowsErrorCode(HRESULT code)
{
    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        (DWORD)code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&messageBuffer,
        0,
        NULL);
    messageBuffer[size - 1] = 0;
    return { messageBuffer, size };
}

std::mutex g_FactoryGuard;
DXGIFactory DXGIFactory::s_Instance = DXGIFactory();

DXGIFactory& DXGIFactory::GetFactory()
{
    if (s_Instance.m_Factory == nullptr) {
        std::lock_guard<std::mutex> guard(g_FactoryGuard);
        if (s_Instance.m_Factory == nullptr) {
            s_Instance.m_Factory = DXGIFactory::CreateDXGIFactory();
        }
    }
    return s_Instance;
}

IDXGIFactory7* DXGIFactory::operator->() const
{
    return this->m_Factory.Get();
}

DXGIFactory::operator IDXGIFactory7*() const
{
    return this->m_Factory.Get();
}

ComPtr<IDXGISwapChain4> DXGIFactory::CreateSwapchain(IUnknown* device, WindowHandle window, SwapchainParams params)
{
    WindowingSystem& windowSystem = WindowingSystem::GetInstance();
    HWND windowHandle = (HWND)windowSystem.GetNativeWindowHandle(window);
    Resolution windowResolution = windowSystem.GetWindowResolution(window);

    ComPtr<IDXGISwapChain4> swapchain = nullptr;
    ComPtr<IDXGISwapChain1> intermediateSwapchain = nullptr;
    const DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {
        .Width = windowResolution.Width,
        .Height = windowResolution.Height,
        .Format = (DXGI_FORMAT)params.Format,
        .Stereo = false,
        .SampleDesc = {
            .Count = 1,
            .Quality = 0,
        },
        .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
        .BufferCount = params.BackbufferCount,
        .Scaling = DXGI_SCALING_STRETCH,
        .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
        .AlphaMode = DXGI_ALPHA_MODE_IGNORE,
        .Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING,
    };
    const DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenDesc = {
        .RefreshRate = {
            .Numerator = 144,
            .Denominator = 1,
        },
        .ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
        .Scaling = DXGI_MODE_SCALING_STRETCHED,
        .Windowed = true,
    };
    DXGI_CALL(this->m_Factory->CreateSwapChainForHwnd(
                  device,
                  windowHandle,
                  &swapchainDesc,
                  &fullscreenDesc,
                  nullptr,
                  intermediateSwapchain.GetAddressOf()),
        "Failed to create swapchain");
    WIN_CALL(intermediateSwapchain->QueryInterface(IID_PPV_ARGS(&swapchain)), "Failed to upcast swapchain to version 4");

    return swapchain;
}

ComPtr<IDXGIFactory7> DXGIFactory::CreateDXGIFactory()
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

std::vector<std::string> DXGIFactory::GetDXGIErrorMessages()
{
    std::vector<std::string> errorMessages;

    ComPtr<IDXGIInfoQueue> infoQueue = nullptr;
    WIN_CALL(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&infoQueue)), "Failed to get DXGI Info Queue");

    const std::uint64_t messageCount = infoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL);
    for (size_t i = 0; i < messageCount; ++i) {
        size_t messageLength = 0;
        WIN_CALL(infoQueue->GetMessage(DXGI_DEBUG_ALL, i, nullptr, &messageLength), "Failed to get message length");

        DXGI_INFO_QUEUE_MESSAGE* message = (DXGI_INFO_QUEUE_MESSAGE*)malloc(messageLength);
        WIN_CALL(infoQueue->GetMessage(DXGI_DEBUG_ALL, i, message, &messageLength), "Failed to get dxgi error message contents");

        errorMessages.emplace_back(message->pDescription, message->DescriptionByteLength);
        free(message);
    }

    return errorMessages;
}

void DXGIFactory::DumpDXGIErrorMessages()
{
    std::vector<std::string> errorMessages = GetDXGIErrorMessages();
    for (const auto& message : errorMessages) {
        LOG_ERROR("{}", message);
    }
}

}
