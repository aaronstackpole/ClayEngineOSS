#pragma once

#ifndef PCH_H
#define PCH_H

#pragma warning(disable : 4619 4616 4061 4265 4365 4571 4623 4625 4626 4628 4668 4710 4711 4746 4774 4820 4987 5026 5027 5031 5032 5039 5045 5219 26812)
#pragma warning(disable : 4471 4917 4986 5029)
#pragma warning(disable : 4643 5043)

#pragma region Win32 API
#ifndef _WIN32_WINNT_WIN10
#define _WIN32_WINNT_WIN10 0x0A00
#endif

#ifndef WINAPI_FAMILY_GAMES
#define WINAPI_FAMILY_GAMES 6
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#pragma warning(push)
#pragma warning(disable : 4005)
#define NOMINMAX
#define NODRAWTEXT
#define NOGDI
#define NOBITMAP
#define NOMCX
#define NOSERVICE
#define NOHELP
#pragma warning(pop)

#include <Windows.h>

#pragma warning(push)
#pragma warning(disable : 4467 5038 5204 5220)
#include <wrl.h>
#pragma warning(pop)

#include <malloc.h>

#include <wincodec.h>

#include <cstddef>
#include <cstdint>
#include <cstdio>

#include <stdexcept>
#pragma endregion

#pragma region WinSock2 API
#include <WinSock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>
#pragma endregion

#pragma region DirectX 11 API
#define _XM_NO_XMVECTOR_OVERLOADS_

#ifdef _GAMING_XBOX
#error This version of DirectX Tool Kit not supported for GDK
#elif defined(_XBOX_ONE) && defined(_TITLE)
#include <d3d11_x.h>
#else
#include <d3d11_1.h>
#endif

#if defined(NTDDI_WIN10_RS2)
#include <dxgi1_6.h>
#else
#include <dxgi1_5.h>
#endif

#ifdef _DEBUG
#include <dxgidebug.h>
#endif

#include <DirectXMath.h>
#include <DirectXColors.h>
#include <DirectXPackedVector.h>
#include <DirectXCollision.h>
#include <DirectXHelpers.h>
#pragma endregion

#pragma region STL
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#include <memory>

#include <type_traits>
#include <algorithm>
#include <utility>
#include <mutex>

#pragma warning(push)
#pragma warning(disable : 4702)
#include <functional>
#pragma warning(pop)

// Containers
#include <tuple>
#include <array>
#include <vector>
#include <list>
#include <map>
#include <set>

#include <chrono>

#include <thread>
#include <future>

#include <string>
#include <iostream>
#include <iomanip>
#include <codecvt> // Deprecated

#include <exception>
#pragma endregion

#pragma region DirectX 11 Toolkit API
//#include "SpriteBatch.h"
//#include "SpriteFont.h"
//#include "SimpleMath.h"
//#include "Mouse.h"
//#include "Keyboard.h"
//#include "WICTextureLoader.h"
#pragma endregion
#endif

