#pragma once

#include "WindowClass.h"
#include "RenderSystem.h"
#include "Ticker.inl"

#include "StepTimer.h"

class App final : public ClayEngine::IDeviceCallback
{
    bool is_resizing = false;

    ClayEngine::WindowClassPtr window;
    ClayEngine::RenderSystemPtr render;
    ClayEngine::TickThreadPtr ticker;

    DX::StepTimer m_timer; //TODO: Migrate to WIP ClayEngine STL Timer.h

    void _Update(DX::StepTimer const& timer);
    void _Draw();

    void _Create_device_dependent_resources();
    void _Create_window_size_dependent_resources();

public:
    App(ClayEngine::WindowClassPtr wndPtr) noexcept(false);
    App(App&&) = default;
    App& operator= (App&&) = default;
    App(App const&) = delete;
    App& operator= (App const&) = delete;
    ~App();

    // Initialization and management
    void SetResizing(bool resizing) { is_resizing = resizing; }
    bool IsResizing() { return is_resizing; }

    // Basic game loop
    void Tick();

    // IDeviceCallback
    void OnDeviceLost() override;
    void OnDeviceRestored() override;

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowMoved();
    void OnWindowSizeChanged(int width, int height);
};
