// (C) 2021 Aaron Stackpole. Portions of this code (C) Microsoft. Used with permission under MIT license.
#pragma once

#ifndef _CLAY_ENGINE_H_
#define _CLAY_ENGINE_H_

#define _CE_BEGIN namespace ClayEngine {
#define _CE_END }

#define _CE_SERVICE_BEGIN namespace ClayEngine { namespace Services {
#define _CE_SERVICE_END } }

#define _CE_NETWORK_BEGIN namespace ClayEngine { namespace Network {
#define _CE_NETWORK_END } }

#define _CE_PLATFORM_BEGIN namespace ClayEngine { namespace Platform {
#define _CE_PLATFORM_END } }

using String = std::wstring;

constexpr LONG default_window_width = 1920;
constexpr LONG default_window_height = 1080;
constexpr auto default_device_lost = true;

//TODO: File system constants loaded here?

inline void PrintHR(std::wstring message, HRESULT hr)
{
    std::wcout << L"[" << std::setfill(L'0') << std::setw(8) << std::this_thread::get_id()
        << L"] " << message << L" HRESULT: 0x" << std::setfill(L'0') << std::setw(8) << std::hex << hr << std::endl;
}

class com_exception : public std::exception
{
    HRESULT result;
public:
    com_exception(HRESULT hr) noexcept : result(hr) {}

    const char* what() const override
    {
        static char s_str[64] = {};
        sprintf_s(s_str, "Failure with HRESULT of %08X", static_cast<unsigned int>(result));
        return s_str;
    }
};

inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw com_exception(hr);
    }
}
#endif
