#include "pch.h"

#pragma warning(disable : 4061)

extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

#include "EngineCore.h"
#include "WindowClass.h"
#include "VoxelFarmThread.h"

using namespace DirectX;
using namespace ClayEngine;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Indicates to hybrid graphics systems to prefer the discrete part by default
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    //std::locale::global(std::locale("en_US.utf-8"));

    if (FAILED(CoInitializeEx(nullptr, COINITBASE_MULTITHREADED))) return -1;
    if (!DirectX::XMVerifyCPUSupport()) return -1;

    // Create default window with 1080 default resolution requested
    auto wnd = std::make_shared<WindowClass>(hInstance, WndProc, L"JAM Prototype - v0.0.1");
    {
        auto hWnd = wnd->GetWindowHandle();
        ShowWindow(hWnd, nCmdShow); // WS_OVERLAPPEDWINDOW for fullscreen

        RECT rect;
        GetClientRect(hWnd, &rect); // Win32 call to get real window RECT
        wnd->SetClientRect(&rect);
    }
    auto core = std::make_unique<EngineCore>(wnd);
    SetWindowLongPtr(wnd->GetWindowHandle(), GWLP_USERDATA, reinterpret_cast<LONG_PTR>(core.get()));

    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            // Call the callback with the MSG data, this will handle the message
            DispatchMessage(&msg);
        }
        else
        {
            core->Draw();
        }
    }

    core.reset();
    CoUninitialize();
    wnd.reset();

    return static_cast<int>(msg.wParam);
}

// DispatchMessage causes the system to call the following function with data from the MSG
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static bool s_fullscreen = false; // Set s_fullscreen to true if defaulting to fullscreen

    auto core = reinterpret_cast<EngineCore*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    if (!core) return DefWindowProc(hWnd, message, wParam, lParam);

    switch (message)
    {
    case WM_PAINT:
        PAINTSTRUCT ps;
        (void)BeginPaint(hWnd, &ps);
        (void)EndPaint(hWnd, &ps);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_GETMINMAXINFO:
        if (lParam)
        {
            auto info = reinterpret_cast<MINMAXINFO*>(lParam);
            info->ptMinTrackSize.x = 320;
            info->ptMinTrackSize.y = 200;
        }
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
        if (core) core->HandleMouseEvents(hWnd, message, wParam, lParam); //TODO: For example...
        break;
    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYUP:
        //if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000)
        //{
        //    // Implements the classic ALT+ENTER fullscreen toggle
        //    if (s_fullscreen)
        //    {
        //        SetWindowLongPtr(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
        //        SetWindowLongPtr(hWnd, GWL_EXSTYLE, 0);
        //        int width = 1920;
        //        int height = 1080;
        //        SetWindowPos(hWnd, HWND_TOP, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
        //        ShowWindow(hWnd, SW_SHOWNORMAL);
        //    }
        //    else
        //    {
        //        SetWindowLongPtr(hWnd, GWL_STYLE, 0);
        //        SetWindowLongPtr(hWnd, GWL_EXSTYLE, WS_EX_TOPMOST);
        //        SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
        //        ShowWindow(hWnd, SW_SHOWMAXIMIZED);
        //    }
        //    s_fullscreen = !s_fullscreen;
        //}
        if (core) core->HandleKeyboardEvents(hWnd, message, wParam, lParam); //TODO: For example...
        break;
    case WM_ACTIVATEAPP:
    case WM_POWERBROADCAST:
    case WM_ENTERSIZEMOVE:
    case WM_EXITSIZEMOVE:
    case WM_SIZE:
    case WM_MOVE:
        if (core) { return core->HandleWindowEvents(hWnd, message, wParam, lParam); }; //TODO: For example...
        break;
    case WM_MENUCHAR:
        return MAKELRESULT(0, MNC_CLOSE);
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

void ExitGame() noexcept // Extern from EngineCore
{
    PostQuitMessage(0);
}
