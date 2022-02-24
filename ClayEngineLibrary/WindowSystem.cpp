#include "pch.h"
#include "WindowSystem.h"

using namespace ClayEngine::Platform;

ATOM WindowSystem::MakeWindowClass(WNDPROC fn)
{
	WNDCLASSEXW wcex = {};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.hInstance = m_instance_handle;
	wcex.lpfnWndProc = fn;
	wcex.hIcon = LoadIconW(m_instance_handle, L"IDI_ICON1");
	wcex.hIconSm = LoadIconW(m_instance_handle, L"IDI_ICON1");
	//wcex.hCursor = LoadCursorW(m_instance_handle, L"IDI_ICON2");
	wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszClassName = m_class_name.c_str();

	return RegisterClassExW(&wcex);
}

BOOL ClayEngine::Platform::WindowSystem::MakeWindowInstance(int flags)
{
	m_window_handle = CreateWindowW(m_class_name.c_str(), m_window_name.c_str(), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, GetWindowWidth(), GetWindowHeight(), nullptr, nullptr, m_instance_handle, nullptr);

	if (!m_window_handle) return FALSE;

	ShowWindow(m_window_handle, flags);
	ShowCursor(true);

	UpdateWindow(m_window_handle);
	UpdateWindowSize();

	return TRUE;
}

void ClayEngine::Platform::WindowSystem::Initialize(HINSTANCE instance, int flags, WNDPROC fn, RECT size)
{
	m_instance_handle = instance;
	SetWindowSize(size);
	MakeWindowClass(fn);
	if (!MakeWindowInstance(flags)) throw std::exception("Win32 ERROR: CreateWindow() failed");
	SetWindowTextW(m_window_handle, m_window_name.c_str());
}

void ClayEngine::Platform::WindowSystem::SetWindowSize(RECT size)
{
	m_size = size;
}

void ClayEngine::Platform::WindowSystem::UpdateWindowSize()
{
	GetClientRect(m_window_handle, &m_size);
}

void ClayEngine::Platform::WindowSystem::SetWindowTitle(Unicode title)
{
	m_window_name = title;
	SetWindowTextW(m_window_handle, m_window_name.c_str());
}

const HWND ClayEngine::Platform::WindowSystem::GetWindowHandle()
{
	return m_window_handle;
}

const RECT& ClayEngine::Platform::WindowSystem::GetWindowSize()
{
	return m_size;
}

const int ClayEngine::Platform::WindowSystem::GetWindowWidth()
{
	return m_size.right - m_size.left;
}

const int ClayEngine::Platform::WindowSystem::GetWindowHeight()
{
	return m_size.bottom - m_size.top;
}

void ClayEngine::Platform::WindowSystem::AddOnWindowSizeChangedCallback(Function fn)
{
	s_onwindowsizechanged.push_back(fn);
}

void ClayEngine::Platform::WindowSystem::OnWindowSizeChanged()
{
	for (auto& element : s_onwindowsizechanged) { element(); }
}

void ClayEngine::Platform::WindowSystem::AddOnActivatedCallback(Function fn)
{
	s_onactivated.push_back(fn);
}

void ClayEngine::Platform::WindowSystem::OnActivated()
{
	for (auto& element : s_onactivated) { element(); }
}

void ClayEngine::Platform::WindowSystem::AddOnDeactivatedCallback(Function fn)
{
	s_ondeactivated.push_back(fn);
}

void ClayEngine::Platform::WindowSystem::OnDeactivated()
{
	for (auto& element : s_ondeactivated) { element(); }
}

void ClayEngine::Platform::WindowSystem::AddOnSuspendingCallback(Function fn)
{
	s_onsuspended.push_back(fn);
}

void ClayEngine::Platform::WindowSystem::OnSuspending()
{
	for (auto& element : s_onsuspended) { element(); }
}

void ClayEngine::Platform::WindowSystem::AddOnResumingCallback(Function fn)
{
	s_onresumed.push_back(fn);
}

void ClayEngine::Platform::WindowSystem::OnResuming()
{
	for (auto& element : s_onresumed) { element(); }
}
