#include "dxgi.h"

#include <dxgidebug.h>
#include <mutex>

namespace raftel::dxgi {

std::pair<const char*, size_t> TranslateWindowsErrorCode(HRESULT code)
{
    LPSTR message_buffer = nullptr;
    size_t size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        (DWORD)code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&message_buffer,
        0,
        NULL);
    message_buffer[size - 1] = 0;
    return { message_buffer, size };
}

std::mutex factory_guard;
DXGIFactory DXGIFactory::s_instance = DXGIFactory();

DXGIFactory& DXGIFactory::GetFactory()
{
    if (s_instance.m_factory == nullptr) {
        std::lock_guard<std::mutex> guard(factory_guard);
        if (s_instance.m_factory == nullptr) {
            s_instance.m_factory = DXGIFactory::CreateDXGIFactory();
        }
    }
    return s_instance;
}

IDXGIFactory7* DXGIFactory::operator->() const
{
    return this->m_factory.Get();
}

DXGIFactory::operator IDXGIFactory7*() const
{
    return this->m_factory.Get();
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
    std::vector<std::string> error_messages;

    ComPtr<IDXGIInfoQueue> info_queue = nullptr;
    WIN_CALL(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&info_queue)), "Failed to get DXGI Info Queue");

    const std::uint64_t message_count = info_queue->GetNumStoredMessages(DXGI_DEBUG_ALL);
    for (size_t i = 0; i < message_count; ++i) {
        size_t message_length = 0;
        WIN_CALL(info_queue->GetMessage(DXGI_DEBUG_ALL, i, nullptr, &message_length), "Failed to get message length");

        DXGI_INFO_QUEUE_MESSAGE* message = (DXGI_INFO_QUEUE_MESSAGE*)malloc(message_length);
        WIN_CALL(info_queue->GetMessage(DXGI_DEBUG_ALL, i, message, &message_length), "Failed to get dxgi error message contents");

        error_messages.emplace_back(message->pDescription, message->DescriptionByteLength);
        free(message);
    }

    return error_messages;
}

void DXGIFactory::DumpDXGIErrorMessages()
{
    std::vector<std::string> error_messages = GetDXGIErrorMessages();
    for (const auto& message : error_messages) {
        LOG_ERROR("{}", message);
    }
}

}
