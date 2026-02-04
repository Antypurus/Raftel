#pragma once

#include <logger.h>

#include <dxgi1_6.h>
#include <string>
#include <vector>
#include <wrl.h>

namespace raftel::dxgi {

using namespace Microsoft::WRL;

ComPtr<IDXGIFactory7> GetDXGIFactory();

std::vector<std::string> GetDXGIErrorMessages();

void DumpDXGIErrorMessages();

std::pair<const char*, size_t> TranslateWindowsErrorCode(HRESULT code);

#define CAT_EXPAND(A, B) A##B
#define CAT(A, B) CAT_EXPAND(A, B)

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

#define DXGI_ERROR_MESSAGE_DUMP                                                              \
    {                                                                                        \
        const auto CAT(dxgi_error_messages, __LINE__) = dxgi::GetDXGIErrorMessages();        \
        for (const auto& CAT(dxgi_message, __LINE__) : CAT(dxgi_error_messages, __LINE__)) { \
            LOG_ERROR("{}", CAT(dxgi_message, __LINE__));                                    \
        }                                                                                    \
    }
#define DXGI_CALL(function_call, error_message, ...) WIN_CALL_GUARD(function_call, error_message, DXGI_ERROR_MESSAGE_DUMP, __VA_ARGS__)

}
