// (C) 2021 Aaron Stackpole. Portions of this code (C) Microsoft. Used with permission under MIT license.
#pragma once

#ifndef _CLAY_ENGINE_H_
#define _CLAY_ENGINE_H_

#define _CE_BEGIN namespace ClayEngine {
#define _CE_END }

#define _CE_NETWORK_BEGIN namespace ClayEngine { namespace Network {
#define _CE_NETWORK_END } }

#include <sstream>
using String = std::wstring;

//TODO: File system constants loaded here?

static const bool default_device_lost = true;
static const LONG default_window_width = 1920L;
static const LONG default_window_height = 1080L;
static const UINT c_flip_present = 0x00000001U;
static const UINT c_allow_tearing = 0x00000002U;
static const UINT c_enable_hdr = 0x00000004U;

inline void WriteLine(String message)
{
    std::wcout << L"[" << std::setfill(L'0') << std::setw(8) << std::this_thread::get_id() << L"] " << message << std::endl;
}
inline void PrintHR(String message, HRESULT hr)
{
    std::wstringstream ss;
    ss << message << L" HRESULT: 0x" << std::setfill(L'0') << std::setw(8) << std::hex << hr;
    WriteLine(ss.str());
}

class com_exception : public std::exception
{
    HRESULT result;

public:
    com_exception(HRESULT hr) noexcept : result(hr) {}

    const char* what() const override
    {
        static char s_str[64] = {};
        sprintf_s(s_str, "COM Exception HRESULT: %08X", static_cast<unsigned int>(result));
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
