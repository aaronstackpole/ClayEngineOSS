#include "pch.h"
#include "App.h"

using namespace DirectX;
using namespace ClayEngine;

using Microsoft::WRL::ComPtr;

extern void ExitGame() noexcept; // Declare extern link to Main.cpp ExitGame() function.
// We're not going to do this, instead we'll have a future from the App thread to call

#pragma region Frame Update and Draw
// Updates the world state with timing
void App::_Update(DX::StepTimer const& timer)
{
    UNREFERENCED_PARAMETER(timer);
    
    //auto elapsed = float(timer.GetElapsedSeconds());
    //...
}

// Renders and presents the scene
void App::_Draw()
{
    if (!render || render->IsDeviceLost()) return;

    // Phase 1: Clear the buffers
    render->PIXBeginEvent(L"Clear");
    auto context = render->GetContextPtr();
    auto rtv = render->GetRTVPtr();
    auto dsv = render->GetDSVPtr();
    context->ClearRenderTargetView(rtv, Colors::CornflowerBlue);
    context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &rtv, dsv);
    auto viewport = render->GetViewport();
    context->RSSetViewports(1, &viewport);
    render->PIXEndEvent();

    // Phase 2: Render the scene
    render->PIXBeginEvent(L"Draw");
    
    //...
    
    render->PIXEndEvent();

    // Phase 3: Present the scene
    render->PIXSetMarker(L"Present");
    render->Present();
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void App::_Create_device_dependent_resources()
{
    //auto device = rs->GetDevicePtr();
    //auto context = rs->GetContextPtr();

    //...
}

// Allocate all memory resources that change on a window SizeChanged event.
void App::_Create_window_size_dependent_resources()
{
    //auto device = rs->GetDevicePtr();
    //auto context = rs->GetContextPtr();

    //...
}
#pragma endregion

App::App(WindowClassPtr wndPtr) noexcept(false)
    : window{ wndPtr }
{
    render = std::make_unique<ClayEngine::RenderSystem>(wndPtr);
    render->RegisterDeviceCallback(this);

    ticker = std::make_unique<TickThread>(this);

    // The following two calls will be redirected to the content system
    _Create_device_dependent_resources();
    _Create_window_size_dependent_resources();

    // Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
        m_timer.SetFixedTimeStep(true);
        m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */
}

App::~App()
{
    ticker.reset();
    render.reset();
}

#pragma region Threaded App Tick
// Executes the basic game loop.
void App::Tick()
{
    m_timer.Tick([&]() { _Update(m_timer); });

    _Draw();
}

void TickThreadFunctor::operator()(std::future<void> future, App* app)
{
    while (future.wait_for(std::chrono::nanoseconds(0)) == std::future_status::timeout)
    {
        if (app && !app->IsResizing())
        {
            app->Tick();
        }
    }
}

TickThread::TickThread(App* app)
{
    t = std::thread{ TickThreadFunctor(), std::move(p.get_future()), app };
}

TickThread::~TickThread()
{
    p.set_value();
    if (t.joinable()) t.join();
}
#pragma endregion

#pragma region Message Handlers
void App::OnActivated()
{
    // App is becoming active window.
}

void App::OnDeactivated()
{
    // App is becoming background window.
}

void App::OnSuspending()
{
    // App is being power-suspended (or minimized).
}

void App::OnResuming()
{
    m_timer.ResetElapsedTime();

    // App is being power-resumed (or returning from minimize).
}

void App::OnWindowMoved()
{
    auto r = window->GetWindowRectPtr();
    render->WindowSizeChanged(r->right, r->bottom);
}

void App::OnWindowSizeChanged(int width, int height)
{
    if (!render->WindowSizeChanged(width, height))
        return;

    _Create_window_size_dependent_resources();

    // App window is being resized.
}

void App::OnDeviceLost()
{
    //...
}

void App::OnDeviceRestored()
{
    _Create_device_dependent_resources();

    _Create_window_size_dependent_resources();
}
#pragma endregion
