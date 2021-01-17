#include "pch.h"
#include "App.h"

#include "WindowClass.h"

using namespace DirectX;
using namespace ClayEngine;

#pragma warning(disable : 4061)

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Indicates to hybrid graphics systems to prefer the discrete part by default
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    
    //std::locale::global(std::locale("en_US.utf-8"));

    if (FAILED(CoInitializeEx(nullptr, COINITBASE_MULTITHREADED))) return -1;
    if (!DirectX::XMVerifyCPUSupport()) return -1;

    // Create Window
    auto wnd = std::make_shared<WindowClass>(hInstance, WndProc);
    {
        auto hWnd = wnd->GetWindowHandle();
        ShowWindow(hWnd, nCmdShow); // WS_OVERLAPPEDWINDOW for fullscreen
        
        auto rec = RECT{};
        GetClientRect(hWnd, &rec); // Win32 call to get real window RECT
        wnd->SetWindowRect(&rec);
    }
    auto app = std::make_unique<App>(wnd);
    SetWindowLongPtr(wnd->GetWindowHandle(), GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app.get()));

    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            // Call the callback with the MSG data, this will handle the message
            DispatchMessage(&msg);
        }
    }

    app.reset();
    CoUninitialize();
    wnd.reset();

    return static_cast<int>(msg.wParam);
}

// DispatchMessage causes the system to call the following function with data from the MSG
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static bool s_in_suspend = false;
    static bool s_minimized = false;
    static bool s_fullscreen = false; // Set s_fullscreen to true if defaulting to fullscreen

    auto app = reinterpret_cast<App*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    if (!app) return DefWindowProc(hWnd, message, wParam, lParam);

    switch (message)
    {
    case WM_PAINT:
        PAINTSTRUCT ps;
        (void)BeginPaint(hWnd, &ps);
        (void)EndPaint(hWnd, &ps);
        break;
    case WM_MOVE:
        if (app) { app->OnWindowMoved(); }
        break;
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
        {
            if (!s_minimized)
            {
                s_minimized = true;
                if (!s_in_suspend && app)
                    app->OnSuspending();
                s_in_suspend = true;
            }
        }
        else if (s_minimized)
        {
            s_minimized = false;
            if (s_in_suspend && app)
                app->OnResuming();
            s_in_suspend = false;
        }
        else if (app && !app->IsResizing())
        {
            app->OnWindowSizeChanged(LOWORD(lParam), HIWORD(lParam));
        }
        break;
    case WM_ENTERSIZEMOVE:
        if (app) app->SetResizing(true);
        break;
    case WM_EXITSIZEMOVE:
        if (app)
        {
            RECT rc;
            GetClientRect(hWnd, &rc);

            app->OnWindowSizeChanged(rc.right - rc.left, rc.bottom - rc.top);
            app->SetResizing(false);
        }
        break;
    case WM_GETMINMAXINFO:
        if (lParam)
        {
            auto info = reinterpret_cast<MINMAXINFO*>(lParam);
            info->ptMinTrackSize.x = 320;
            info->ptMinTrackSize.y = 200;
        }
        break;
    case WM_ACTIVATEAPP:
        if (app)
        {
            if (wParam)
            {
                app->OnActivated();
            }
            else
            {
                app->OnDeactivated();
            }
        }
        break;
    case WM_POWERBROADCAST:
        switch (wParam)
        {
        case PBT_APMQUERYSUSPEND:
            if (!s_in_suspend && app)
                app->OnSuspending();
            s_in_suspend = true;
            return TRUE;
        case PBT_APMRESUMESUSPEND:
            if (!s_minimized)
            {
                if (s_in_suspend && app)
                    app->OnResuming();
                s_in_suspend = false;
            }
            return TRUE;
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
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

                SetWindowPos(hWnd, HWND_TOP, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
                ShowWindow(hWnd, SW_SHOWNORMAL);
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
    case WM_MENUCHAR:
        // A menu is active and the user presses a key that does not correspond
        // to any mnemonic or accelerator key. Ignore so we don't produce an error beep.
        return MAKELRESULT(0, MNC_CLOSE);
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

void ExitGame() noexcept // Extern from Game.obj
{
    PostQuitMessage(0);
}
