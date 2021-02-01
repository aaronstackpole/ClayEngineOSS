#pragma once

#include "Timer.h"
#include "Services.h"
#include "WindowClass.h"
#include "RenderSystem.h"
#include "ContentSystem.h"
#include "VoxelFarmThread.h"

#pragma region TickerThread Interface
class EngineCore;
_CE_BEGIN
struct TickerThreadFunctor
{
    void operator()(std::future<void> future, EngineCore* app);
};
class TickerThread
{
    std::thread t;
    std::promise<void> p{};

public:
    TickerThread(EngineCore* app);
    ~TickerThread();
};
using TickerThreadPtr = std::unique_ptr<ClayEngine::TickerThread>;
_CE_END
#pragma endregion

class EngineCore final : public ClayEngine::IDeviceCallback
{
    bool is_minimized = false;
    bool is_resizing = false;
    bool is_suspended = false;

    ClayEngine::Timer time;

    ClayEngine::WindowClassPtr window;
    ClayEngine::ServicesPtr services;
    ClayEngine::TickerThreadPtr ticker;
    ClayEngine::RenderSystemPtr render;
    ClayEngine::ContentSystemPtr content;
    ClayEngine::VoxelFarmThreadPtr vf;

    void _Update(ClayEngine::Timer const& timer);
    void _Render();

    void _Create_device_dependent_resources();
    void _Create_window_size_dependent_resources();

public:
    EngineCore(ClayEngine::WindowClassPtr wndPtr) noexcept(false);
    EngineCore(EngineCore&&) = default;
    EngineCore& operator=(EngineCore&&) = default;
    EngineCore(EngineCore const&) = delete;
    EngineCore& operator=(EngineCore const&) = delete;
    ~EngineCore();

    const ClayEngine::Services* GetServices() { return services.get(); }

    // Engine core loops, Tick->_Update, Draw->_Render
    // IsWindowReady is a rudimentary mutex for the future state system
    bool IsWindowReady() const noexcept { return !is_minimized && !is_suspended && !is_resizing; }
    void Tick();
    void Draw();

    // IDeviceCallback
    void OnDeviceLost() override;
    void OnDeviceRestored() override;

    // Message pump events
    void OnActivated(); //TODO: Start calling draw again if we restore...
    void OnDeactivated(); //TODO: Stop calling draw if we minimize...
    void OnSuspending();
    void OnResuming();
    void OnWindowMoved();
    void OnWindowResized(LONG width, LONG height);

    // Following are temporary, need to more deeply analyze the window messaging scheme to see 
    // if I can detach engine core from messages between windowclass and rendersystem
    LRESULT HandleWindowEvents(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        // I think there must be a message for maximize and restore that's not being handled here...
        switch (message)
        {
        case WM_MOVE:
            OnWindowMoved();
            break;
        case WM_ENTERSIZEMOVE:
            is_resizing = true;
            break;
        case WM_EXITSIZEMOVE:
            RECT rect;
            GetClientRect(hWnd, &rect);
            window->SetClientRect(&rect);
            OnWindowResized(rect.right - rect.left, rect.bottom - rect.top);
            is_resizing = false;
            break;
        case WM_ACTIVATEAPP:
            if (wParam)
            {
                OnActivated();
            }
            else
            {
                OnDeactivated();
            }
            break;
        case WM_SIZE:
            if (wParam == SIZE_MINIMIZED)
            {
                if (!is_minimized)
                {
                    is_minimized = true;

                    // We need the is_suspended flag set before we call OnSuspending to keep the TickerThread in line
                    // So we cache it here first, then set it, then if needed, call OnSuspending.
                    auto is = is_suspended;
                    is_suspended = true;
                    if (!is)
                    {
                        OnSuspending();
                    }
                }
            }
            else if (is_minimized)
            {
                if (is_suspended)
                {
                    OnResuming();
                }
                is_suspended = false;
                is_minimized = false;
            }
            else if (!is_resizing)
            {
                OnWindowResized(LOWORD(lParam), HIWORD(lParam));
            }
            break;
        case WM_POWERBROADCAST:
            switch (wParam)
            {
            case PBT_APMQUERYSUSPEND:
                if (!is_suspended)
                {
                    OnSuspending();
                }
                is_suspended = true;
                return TRUE;
            case PBT_APMRESUMESUSPEND:
                if (!is_minimized)
                {
                    if (is_suspended)
                    {
                        OnResuming();
                    }
                    is_suspended = false;
                }
                return TRUE;
            }
            break;
        }
        return 0;
    }
    LRESULT HandleKeyboardEvents(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        return 0;
    }
    LRESULT HandleMouseEvents(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        return 0;
    }
};
