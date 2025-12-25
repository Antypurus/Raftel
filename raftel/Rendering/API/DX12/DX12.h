#pragma once

#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include <logger.h>

namespace raftel {
static std::pair<const char*, size_t> TranslateWindowErrorCode(HRESULT code)
{
    LPSTR message_buffer = nullptr;
    size_t size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&message_buffer,
        0,
        NULL);
    message_buffer[size - 1] = 0;
    return { message_buffer, size };
}
}

#define WIN_CALL(function_call, error_message, ...)                                                             \
    {                                                                                                           \
        HRESULT result##__LINE__ = function_call;                                                               \
        if (result##__LINE__ != S_OK) {                                                                         \
            auto error_message_##__LINE__ = raftel::TranslateWindowErrorCode(result##__LINE__);                 \
            LOG_ERROR("{}", std::string_view(error_message_##__LINE__.first, error_message_##__LINE__.second)); \
            LocalFree((LPSTR)error_message_##__LINE__.first);                                                   \
            LOG_ERROR(error_message, __VA_ARGS__);                                                              \
            __debugbreak();                                                                                     \
            exit(-1);                                                                                           \
        }                                                                                                       \
    }
