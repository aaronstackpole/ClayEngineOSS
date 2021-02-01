#include "pch.h"
#include "WindowClass.h"

ClayEngine::WindowClass::WindowClass(HINSTANCE hInstance, WNDPROC fn, String title)
{
    if (AllocConsole())
    {
        FILE* file = nullptr;
        _wfreopen_s(&file, L"CONIN$", L"r", stdin);
        _wfreopen_s(&file, L"CONOUT$", L"w", stdout);
        _wfreopen_s(&file, L"CONOUT$", L"w", stderr);
        WriteLine(L"Allocated debug console");
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

    hWnd = CreateWindowExW(0, L"ClayEngineWindowClass", title.c_str(), WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, GetWindowWidth(), GetWindowHeight(), nullptr, nullptr, hInstance,
        nullptr);
    // TODO: Change to CreateWindowExW(WS_EX_TOPMOST, L"$safeprojectname$WindowClass", L"$projectname$", WS_POPUP,
    // to default to fullscreen.
    if (!hWnd) throw;
}

void ClayEngine::WindowClass::SetClientRect(const LPRECT rectPtr)
{
    rect.reset();
    rect = nullptr;
    rect = std::make_unique<RECT>(*rectPtr);
}
