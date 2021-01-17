#include "pch.h"
#include "RenderSystem.h"

using namespace DirectX;
using namespace ClayEngine;

using Microsoft::WRL::ComPtr;

#pragma warning(disable : 4061)

inline DXGI_FORMAT NoSRGB(DXGI_FORMAT fmt) noexcept
{
    switch (fmt)
    {
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:   return DXGI_FORMAT_R8G8B8A8_UNORM;
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:   return DXGI_FORMAT_B8G8R8A8_UNORM;
    case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:   return DXGI_FORMAT_B8G8R8X8_UNORM;
    default:                                return fmt;
    }
}

// Constructor for DeviceResources.
RenderSystem::RenderSystem(WindowClassPtr wndPtr) noexcept
    : wnd{ wndPtr }
    , i_device_callback(nullptr)
{
    res = std::make_unique<ClayEngine::DX11Resources>();
    pipe = std::make_unique<ClayEngine::DX11Pipeline>(
        wnd->GetWindowHandle(),
        res->GetParams(),
        wnd->GetWindowWidth(),
        wnd->GetWindowHeight()
        );

    // In order to communicate with the functor, we need to pass lambda callbacks to functions in this class.
    // Create an OnWhateverEvent with std::function parameter for calls from thread (use promise for async callbacks)
    t = std::thread{ ClayEngine::RenderSystemSentinel(), std::move(p.get_future()), this };
}

RenderSystem::~RenderSystem()
{
    p.set_value();
    if (t.joinable()) t.join();

    pipe.reset();
    res.reset();
}

void ClayEngine::RenderSystem::ResetRenderSystem()
{
    m.lock();
    // Calls the App letting it know to destroy content objects
    if (i_device_callback) { i_device_callback->OnDeviceLost(); }

    pipe.reset();
    res.reset();

    res = std::make_unique<ClayEngine::DX11Resources>();
    pipe = std::make_unique<ClayEngine::DX11Pipeline>(
        wnd->GetWindowHandle(),
        res->GetParams(),
        wnd->GetWindowWidth(),
        wnd->GetWindowHeight()
        );

    // Calls the App letting it know to create the content objects
    if (i_device_callback) { i_device_callback->OnDeviceRestored(); }
    m.unlock();
}

// This method is called when the Win32 window changes size
bool RenderSystem::WindowSizeChanged(UINT width, UINT height)
{
    m.lock();
    auto result = pipe->WindowSizeChanged(width, height);
    m.unlock();

    return result;
}

// Present the contents of the swap chain to the screen.
void RenderSystem::Present()
{
    m.lock();
    HRESULT hr = E_FAIL;
    if (res->GetDeviceOptions() & ClayEngine::c_allow_tearing)
    {
        // Recommended to always use tearing if supported when using a sync interval of 0.
        hr = pipe->GetSwapChainPtr()->Present(0, DXGI_PRESENT_ALLOW_TEARING);
    }
    else
    {
        // The first argument instructs DXGI to block until VSync, putting the application
        // to sleep until the next VSync. This ensures we don't waste any cycles rendering
        // frames that will never be displayed to the screen.
        hr = pipe->GetSwapChainPtr()->Present(1, 0);
    }

    // Discard the contents of the render target.
    // This is a valid operation only when the existing contents will be entirely
    // overwritten. If dirty or scroll rects are used, this call should be removed.
    res->GetContextPtr()->DiscardView(pipe->GetRTVPtr());

    if (pipe->GetDSVPtr())
    {
        // Discard the contents of the depth stencil.
        res->GetContextPtr()->DiscardView(pipe->GetDSVPtr());
    }

    // If the device was removed either by a disconnection or a driver upgrade, we
    // must recreate all device resources.
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
    {
#ifdef _DEBUG
        PrintHR(L"Device Lost on Present", static_cast<unsigned int>((hr == DXGI_ERROR_DEVICE_REMOVED) ? res->GetDevicePtr()->GetDeviceRemovedReason() : hr));
#endif
        pipe->SetDeviceLost();
    }
    else
    {
        ThrowIfFailed(hr);

        if (!res->GetFactoryPtr()->IsCurrent())
        {
            // Output information is cached on the DXGI Factory. If it is stale we need to create a new factory.
            res->ReconstructFactory();
        }
    }
    m.unlock();
}

void ClayEngine::RenderSystemSentinel::operator()(std::future<void> future, ClayEngine::RenderSystem* res)
{
    while (future.wait_for(std::chrono::nanoseconds(0)) == std::future_status::timeout)
    {
        // Objective of this module is to keep a DX11 device and swapchain/pipeline interfaces running
        if (!res->IsDeviceLost() && last_device_lost) // Pipeline constructed, we have not handled it
        {
            last_device_lost = res->IsDeviceLost();
        }

        if (res->IsDeviceLost() && !last_device_lost) // Device was constructed, but has been lost
        {
            res->ResetRenderSystem();
            last_device_lost = res->IsDeviceLost();
        }
    }
}
