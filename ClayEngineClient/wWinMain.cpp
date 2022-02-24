#include "pch.h"

#include "ClayEngine.h"
#include "WindowSystem.h"
#include "InputSystem.h"

#include "ClientCoreSystem.h"

#include "Mouse.h"

#include "SquareChase.h"

using namespace ClayEngine;
using namespace ClayEngine::Platform;

namespace
{
	SettingsPtr g_settings = nullptr;
	ClientCoreSystemPtr g_core = nullptr;
	InputSystemPtr g_input = nullptr;
	SquareChasePtr g_chase = nullptr;
}

// Window callback forward declaration
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Main entry point
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{

	//std::locale::global(std::locale("en_US.utf-8"));

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	if (!PlatformStart()) return -1;
	
	try
	{
		g_settings = std::make_unique<Settings>();
	}
	catch (std::exception ex)
	{
		std::cout << ex.what();
		return -2;
	}

	auto ws = g_settings->GetWindowSettings();
	ws.Width = (ws.Width < c_min_window_width) ? c_min_window_width : ws.Width;
	ws.Height = (ws.Height < c_min_window_height) ? c_min_window_height : ws.Height;

	WindowSystem::Initialize(hInstance, nShowCmd, WndProc, RECT{ 0, 0, ws.Width, ws.Height });
	WindowSystem::SetWindowTitle(L"ClayEngine Client Prototype - Milestone 0.0.2");

	g_input = Services::MakeService<InputSystem>();
	g_core = Services::MakeService<ClientCoreSystem>();

	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (g_core->GetStateChanged())
		{
			g_core->OnStateChanged();
		}

		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	PlatformStop();
	return static_cast<int>(msg.wParam);
}

// Window callback implementation
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static bool m_in_sizemove = false;
	static bool m_in_suspend = false;
	static bool m_minimized = false;
	static bool m_fullscreen = false;

	switch (message)
	{
	case WM_MENUCHAR: // Supress the menu
		return MAKELRESULT(0, MNC_CLOSE);
	case WM_PAINT:
		if (!m_in_sizemove)
		{
			PAINTSTRUCT ps;
			(void)BeginPaint(hWnd, &ps);
			//HDC hdc = BeginPaint(hWnd, &ps);
			//TODO: Add any drawing code that uses hdc here...
			EndPaint(hWnd, &ps);
		}
		break;
	case WM_GETMINMAXINFO:
		if (lParam)
		{
			auto info = reinterpret_cast<MINMAXINFO*>(lParam);
			info->ptMinTrackSize.x = c_min_window_width;
			info->ptMinTrackSize.y = c_min_window_height;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_SYSKEYUP:
	case WM_KEYUP:
		if (g_input) g_input->OnKeyUp(wParam, lParam);
		break;
	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
		if (g_input) g_input->OnKeyDown(wParam, lParam);
		break;
	case WM_CHAR:
		if (g_input) g_input->OnChar(wParam, lParam);
		break;
	case WM_INPUT:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEHOVER:
		DirectX::Mouse::ProcessMessage(message, wParam, lParam); //if (g_input) g_input->OnMouseEvent(message, wParam, lParam);
		break;
	case WM_ACTIVATEAPP:
		DirectX::Mouse::ProcessMessage(message, wParam, lParam); //if (g_input) g_input->OnMouseEvent(message, wParam, lParam);
		if (wParam)
		{
			WindowSystem::OnActivated();
		}
		else
		{
			WindowSystem::OnDeactivated();
		}
		break;
	case WM_POWERBROADCAST:
		switch (wParam)
		{
		case PBT_APMQUERYSUSPEND:
			if (!m_in_suspend)
				WindowSystem::OnSuspending();
			m_in_suspend = true;
			return TRUE;
		case PBT_APMRESUMESUSPEND:
			if (!m_minimized)
			{
				if (m_in_suspend)
					WindowSystem::OnResuming();
				m_in_suspend = false;
			}
			return TRUE;
		}
		break;
	case WM_ENTERSIZEMOVE:
		m_in_sizemove = true;
		break;
	case WM_EXITSIZEMOVE:
		m_in_sizemove = false;
		{
			WindowSystem::UpdateWindowSize();
		}
		break;
	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED)
		{
			if (!m_minimized)
			{
				m_minimized = true;
				if (!m_in_suspend)
					WindowSystem::OnSuspending();
				m_in_suspend = true;
			}
		}
		else if (m_minimized)
		{
			m_minimized = false;
			if (m_in_suspend)
				WindowSystem::OnResuming();
			m_in_suspend = false;
		}
		else if (!m_in_sizemove)
		{
			WindowSystem::UpdateWindowSize();
		}
		break;
	//case WM_MOVE:
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}


/*
case WM_SYSKEYDOWN:
	if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000)
	{
		// Implements the classic ALT+ENTER fullscreen toggle
		if (s_fullscreen)
		{
			SetWindowLongPtr(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
			SetWindowLongPtr(hWnd, GWL_EXSTYLE, 0);

			int width = 800;
			int height = 600;
			if (game)
				game->GetDefaultSize(width, height);

			ShowWindow(hWnd, SW_SHOWNORMAL);

			SetWindowPos(hWnd, HWND_TOP, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
		}
		else
		{
			SetWindowLongPtr(hWnd, GWL_STYLE, 0);
			SetWindowLongPtr(hWnd, GWL_EXSTYLE, WS_EX_TOPMOST);

			SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

			ShowWindow(hWnd, SW_SHOWMAXIMIZED);
		}

		s_fullscreen = !s_fullscreen;
	}
	break;
*/
