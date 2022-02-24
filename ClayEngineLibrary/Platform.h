#pragma once
/******************************************************************************/
/*                                                                            */
/* ClayEngine Platform Header (C) 2022 Epoch Meridian, LLC.                   */
/*                                                                            */
/*                                                                            */
/******************************************************************************/

#include <string>
#include <exception>
#include <Windows.h>
#include <DirectXMath.h>

namespace ClayEngine
{
	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			// Set a breakpoint on this line to catch DirectX API errors
			throw std::exception();
		}
	}

	inline void ThrowIfFailed(HRESULT hr, std::string reason)
	{
		if (FAILED(hr))
		{
			// Set a breakpoint on this line to catch DirectX API errors
			throw std::exception(reason.c_str());
		}
	}

	inline bool PlatformStart()
	{
		if (AllocConsole())
		{
			FILE* file = nullptr;
			_wfreopen_s(&file, TEXT("CONIN$"), TEXT("r"), stdin);
			_wfreopen_s(&file, TEXT("CONOUT$"), TEXT("w"), stdout);
			_wfreopen_s(&file, TEXT("CONOUT$"), TEXT("w"), stderr);
			WriteLine(TEXT("PlatformStart INFO: Allocated default console"));
		}
		else return false;

		if (FAILED(CoInitializeEx(nullptr, COINITBASE_MULTITHREADED))) return false;
		if (!DirectX::XMVerifyCPUSupport()) return false;

		return true;
	}

	inline void PlatformStop()
	{
		CoUninitialize();
	}

}