#pragma once

#include <Windowing/Window.h>
#include <cstdint>
#include <logger.h>

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
    std::uint8_t backbuffer_count = 2;
    ResourceFormat format = ResourceFormat::BGRA8Unorm;
};

class DXGIFactory {
private:
    static DXGIFactory s_instance;
    ComPtr<IDXGIFactory7> m_factory = nullptr;

public:
    static DXGIFactory& GetFactory();

    IDXGIFactory7* operator->() const;
    operator IDXGIFactory7*() const;

    ComPtr<IDXGISwapChain4> CreateSwapchain(IUnknown* device, WindowHandle window, SwapchainParams params = {});
    std::vector<std::string> GetDXGIErrorMessages();
    void DumpDXGIErrorMessages();

private:
    DXGIFactory() = default;
    static ComPtr<IDXGIFactory7> CreateDXGIFactory();
};

#define CAT_EXPAND(A, B) A##B
#define CAT(A, B) CAT_EXPAND(A, B)

std::pair<const char*, size_t> TranslateWindowsErrorCode(HRESULT code);
#define WIN_CALL_GUARD(function_call, error_message, error_code, ...)                                                     \
    {                                                                                                                     \
        HRESULT CAT(result, __LINE__) = (function_call);                                                                  \
        if (CAT(result, __LINE__) != S_OK && CAT(result, __LINE__) != S_FALSE) {                                          \
            auto CAT(error_message_, __LINE__) = raftel::dxgi::TranslateWindowsErrorCode(CAT(result, __LINE__));          \
            LOG_ERROR("{}", std::string_view(CAT(error_message_, __LINE__).first, CAT(error_message_, __LINE__).second)); \
            LocalFree((LPSTR)CAT(error_message_, __LINE__).first);                                                        \
            LOG_ERROR(error_message, __VA_ARGS__);                                                                        \
            {                                                                                                             \
                error_code                                                                                                \
            };                                                                                                            \
            __debugbreak();                                                                                               \
            exit(-1);                                                                                                     \
        }                                                                                                                 \
    }

#define WIN_CALL(function_call, error_message, ...) WIN_CALL_GUARD(function_call, error_message, {}, __VA_ARGS__)

#define DXGI_ERROR_MESSAGE_DUMP                                                                                 \
    {                                                                                                           \
        const auto CAT(dxgi_error_messages, __LINE__) = dxgi::DXGIFactory::GetFactory().GetDXGIErrorMessages(); \
        for (const auto& CAT(dxgi_message, __LINE__) : CAT(dxgi_error_messages, __LINE__)) {                    \
            LOG_ERROR("{}", CAT(dxgi_message, __LINE__));                                                       \
        }                                                                                                       \
    }
#define DXGI_CALL(function_call, error_message, ...) WIN_CALL_GUARD(function_call, error_message, DXGI_ERROR_MESSAGE_DUMP, __VA_ARGS__)

}
