#pragma once

#include "ClayEngine.h"

_CE_BEGIN
using RectPtr = std::unique_ptr<RECT>;

class WindowClass
{
    HWND hWnd = nullptr;
    WNDCLASSEXW wcex{};
    RectPtr rect;

public:
    WindowClass(HINSTANCE hInstance, WNDPROC fn)
    {
        if (AllocConsole())
        {
            FILE* file = nullptr;
            _wfreopen_s(&file, L"CONIN$", L"r", stdin);
            _wfreopen_s(&file, L"CONOUT$", L"w", stdout);
            _wfreopen_s(&file, L"CONOUT$", L"w", stderr);
            std::wcout << L"[" << std::setfill(L'0') << std::setw(8) << std::this_thread::get_id() << L"] Allocated debug console" << std::endl;
        }

        rect = std::make_unique<RECT>(RECT{ 0, 0, default_window_width, default_window_height });

        wcex.cbSize = sizeof(WNDCLASSEXW);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = fn;
        wcex.hInstance = hInstance;
        wcex.hIcon = LoadIconW(hInstance, L"IDI_ICON");
        wcex.hIconSm = LoadIconW(hInstance, L"IDI_ICON");
        wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszClassName = L"ClayEngineWindowClass";
        if (!RegisterClassExW(&wcex)) throw;

        AdjustWindowRect(rect.get(), WS_OVERLAPPEDWINDOW, FALSE);
        ShowCursor(true);

        hWnd = CreateWindowExW(0, L"ClayEngineWindowClass", L"ClayEngine Prototype - v0.0.1", WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, GetWindowWidth(), GetWindowHeight(), nullptr, nullptr, hInstance,
            nullptr);
        // TODO: Change to CreateWindowExW(WS_EX_TOPMOST, L"$safeprojectname$WindowClass", L"$projectname$", WS_POPUP,
        // to default to fullscreen.
        if (!hWnd) throw;
    }
    ~WindowClass() = default;

    const LPRECT GetWindowRectPtr() const
    {
        return rect.get();
    }
    const HWND GetWindowHandle() const
    {
        return hWnd;
    }
    void SetWindowRect(const LPRECT rectPtr)
    {
        rect.reset();
        rect = nullptr;
        rect = std::make_unique<RECT>(*rectPtr);
    }
    int GetWindowWidth()
    {
        return rect->right - rect->left;
    }
    int GetWindowHeight()
    {
        return rect->bottom - rect->top;
    }
};
using WindowClassPtr = std::shared_ptr<WindowClass>;
_CE_END
