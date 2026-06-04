#pragma once

#include <Windowing/Window.h>
#include <core/logger.h>

#include <cstdint>
#include <dxgi1_6.h>
#include <dxgiformat.h>
#include <string>
#include <vector>
#include <wrl.h>

namespace raftel::dxgi {

using namespace Microsoft::WRL;

enum class ResourceFormat {
    // RGBA 8bpc formats
    RGBA8Int = DXGI_FORMAT_R8G8B8A8_SINT,
    RGBA8Uint = DXGI_FORMAT_R8G8B8A8_UINT,
    RGBA8Norm = DXGI_FORMAT_R8G8B8A8_SNORM,
    RGBA8UNorm = DXGI_FORMAT_R8G8B8A8_UNORM,
    RGBA8Typeless = DXGI_FORMAT_R8G8B8A8_TYPELESS,
    RGBA8UNormSRGB = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
    // BGRA 8bpc formats
    BGRA8Unorm = DXGI_FORMAT_B8G8R8A8_UNORM,
    BGRA8UNormSRGB = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
    BGRA8Typeless = DXGI_FORMAT_B8G8R8A8_TYPELESS,
    // Depth-Stencil formats
    D24UnormS8Uint = DXGI_FORMAT_D24_UNORM_S8_UINT,
};

struct SwapchainParams {
    std::uint8_t BackbufferCount = 2;
    ResourceFormat Format = ResourceFormat::BGRA8Unorm;
};

class DXGIFactory {
private:
    static DXGIFactory s_Instance;
    ComPtr<IDXGIFactory7> m_Factory = nullptr;

public:
    static DXGIFactory& GetFactory();

    IDXGIFactory7* operator->() const;
    operator IDXGIFactory7*() const;

    ComPtr<IDXGISwapChain4> CreateSwapchain(IUnknown* Device, WindowHandle Window, SwapchainParams Params = { });
    std::vector<std::string> GetDXGIErrorMessages();
    void DumpDXGIErrorMessages();

private:
    DXGIFactory() = default;
    static ComPtr<IDXGIFactory7> CreateDXGIFactory();
};

#define CAT_EXPAND(A, B) A##B
#define CAT(A, B) CAT_EXPAND(A, B)

std::pair<const char*, size_t> TranslateWindowsErrorCode(HRESULT Code);
#define WIN_CALL_GUARD(p_FunctionCall, p_ErrorMessage, p_ErrorCode, ...)                                                \
    {                                                                                                                   \
        HRESULT CAT(result, __LINE__) = (p_FunctionCall);                                                               \
        if (CAT(result, __LINE__) != S_OK && CAT(result, __LINE__) != S_FALSE) {                                        \
            auto CAT(errorMessage_, __LINE__) = raftel::dxgi::TranslateWindowsErrorCode(CAT(result, __LINE__));         \
            LOG_ERROR("{}", std::string_view(CAT(errorMessage_, __LINE__).first, CAT(errorMessage_, __LINE__).second)); \
            LocalFree((LPSTR)CAT(errorMessage_, __LINE__).first);                                                       \
            LOG_ERROR(p_ErrorMessage, __VA_ARGS__);                                                                     \
            {                                                                                                           \
                p_ErrorCode                                                                                             \
            };                                                                                                          \
            __debugbreak();                                                                                             \
            exit(-1);                                                                                                   \
        }                                                                                                               \
    }

#define WIN_CALL(p_FunctionCall, p_ErrorMessage, ...) WIN_CALL_GUARD(p_FunctionCall, p_ErrorMessage, { }, __VA_ARGS__)

#define DXGI_ERROR_MESSAGE_DUMP                                                                               \
    {                                                                                                         \
        const auto CAT(dxgiErrorMessages, __LINE__) = dxgi::DXGIFactory::GetFactory().GetDXGIErrorMessages(); \
        for (const auto& CAT(dxgiMessage, __LINE__) : CAT(dxgiErrorMessages, __LINE__)) {                     \
            LOG_ERROR("{}", CAT(dxgiMessage, __LINE__));                                                      \
        }                                                                                                     \
    }
#define DXGI_CALL(p_FunctionCall, p_ErrorMessage, ...) WIN_CALL_GUARD(p_FunctionCall, p_ErrorMessage, DXGI_ERROR_MESSAGE_DUMP, __VA_ARGS__)

}
