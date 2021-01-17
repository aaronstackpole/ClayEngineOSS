#pragma once

#include "WindowClass.h"
#include "DX11Resources.h"
#include "DX11Pipeline.h"

#include <mutex>
#include <thread>
#include <future>

_CE_BEGIN
interface IDeviceCallback
{
    virtual void OnDeviceLost() = 0;
    virtual void OnDeviceRestored() = 0;

protected:
    ~IDeviceCallback() = default;
};

// Manages DirectX device resources and rendering pipeline
class RenderSystem
{
    std::mutex m;
    std::thread t;
    std::promise<void> p{};

    WindowClassPtr wnd;
    DX11ResourcesPtr res;
    DX11PipelinePtr pipe;

    IDeviceCallback* i_device_callback;

public:
    RenderSystem(WindowClassPtr wndPtr) noexcept(true);
    RenderSystem(RenderSystem&&) = default;
    RenderSystem& operator= (RenderSystem&&) = default;
    RenderSystem(RenderSystem const&) = delete;
    RenderSystem& operator= (RenderSystem const&) = delete;
    ~RenderSystem();

    void ResetRenderSystem();

    bool WindowSizeChanged(UINT width, UINT height);
    bool IsDeviceLost() { return pipe->IsDeviceLost(); }
    void RegisterDeviceCallback(IDeviceCallback* deviceNotify) noexcept { i_device_callback = deviceNotify; }
        
    void Present();

    auto GetDevicePtr() const { return res->GetDevicePtr(); }
    auto GetContextPtr() const { return res->GetContextPtr(); }
    RECT GetOutputRect() const noexcept { return pipe->GetOutputRect(); }
    D3D11_VIEWPORT GetViewport() const noexcept { return pipe->GetViewport(); }
    //UINT                    GetDeviceOptions() const noexcept { return res->GetDeviceOptions(); }
    //D3D_FEATURE_LEVEL       GetDeviceFeatureLevel() const noexcept { return res->GetFeatureLevel(); }
    //auto                    GetSwapChain() const noexcept { return pipe->GetSwapChainPtr(); }
    auto GetRenderTarget() const { return pipe->GetRenderTargetPtr(); }
    auto GetDepthStencil() const { return pipe->GetDepthStencilPtr(); }
    auto GetRTVPtr() const { return pipe->GetRTVPtr(); }
    auto GetDSVPtr() const { return pipe->GetDSVPtr(); }
    //DXGI_FORMAT             GetRenderTargetFormat() const noexcept { return pipe->GetRTVFmt(); }
    //DXGI_FORMAT             GetDepthBufferFormat() const noexcept { return pipe->GetDSVFmt(); }
    //UINT                    GetBufferCount() const noexcept { return pipe->GetBufferCount(); }
    DXGI_COLOR_SPACE_TYPE   GetColorSpace() const noexcept { return pipe->GetColorSpace(); }

    // Performance events
    void PIXBeginEvent(_In_z_ const wchar_t* name)
    {
        res->GetAnnotationPtr()->BeginEvent(name);
    }
    void PIXEndEvent()
    {
        res->GetAnnotationPtr()->EndEvent();
    }
    void PIXSetMarker(_In_z_ const wchar_t* name)
    {
        res->GetAnnotationPtr()->SetMarker(name);
    }
};
using RenderSystemPtr = std::unique_ptr<RenderSystem>;

struct RenderSystemSentinel
{
    void operator()(std::future<void> future, ClayEngine::RenderSystem* res);
private:
    bool last_device_lost = default_device_lost;
};
_CE_END