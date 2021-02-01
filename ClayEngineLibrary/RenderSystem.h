#pragma once

#include "WindowClass.h"
#include "Services.h"

#include <mutex>
#include <thread>
#include <future>

_CE_BEGIN
/// <summary>
/// DX11Resources class provides DirectX 11 hardware interfaces
/// </summary>
static const D3D_FEATURE_LEVEL s_feature_levels[] = {
    D3D_FEATURE_LEVEL_11_1,
    D3D_FEATURE_LEVEL_11_0,
    D3D_FEATURE_LEVEL_10_1,
    D3D_FEATURE_LEVEL_10_0,
    D3D_FEATURE_LEVEL_9_3,
    D3D_FEATURE_LEVEL_9_2,
    D3D_FEATURE_LEVEL_9_1
};

using FactoryPtr = Microsoft::WRL::ComPtr<IDXGIFactory2>;
using DevicePtr = Microsoft::WRL::ComPtr<ID3D11Device1>;
using ContextPtr = Microsoft::WRL::ComPtr<ID3D11DeviceContext1>;
using AnnoPtr = Microsoft::WRL::ComPtr<ID3DUserDefinedAnnotation>;

// This is a helper hack to facilitate the refactoring of the DX11Pipeline object, to be removed...
class DX11Resources;
struct Resources
{
    UINT DeviceOptions;
    IDXGIFactory2* DXGIFactoryPtr;
    ID3D11Device1* D3D11DevicePtr;
    ID3D11DeviceContext1* D3D11ContextPtr;
    ID3DUserDefinedAnnotation* D3DAnnotationPtr;
    DX11Resources* DX11ResourcesPtr;
};

class DX11Resources
{
    FactoryPtr fac;
    DevicePtr dev;
    ContextPtr ctx;
    AnnoPtr anno;

    UINT creation_flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    D3D_FEATURE_LEVEL min_feature_level; // Defined minimum feature level required
    D3D_FEATURE_LEVEL feature_level; // Holds feature levels of device created
    UINT feature_level_count = 0;
    UINT device_options = 0;

#ifdef _DEBUG
    bool _Is_debug_device_available() noexcept;
#endif
    void _Create_factory_and_flags();
    void _Validate_device_options();
    void _Create_device_and_context();

public:
    DX11Resources(UINT flags = c_flip_present, D3D_FEATURE_LEVEL minFeatureLevel = D3D_FEATURE_LEVEL_10_0);
    DX11Resources(DX11Resources&&) = default;
    DX11Resources& operator=(DX11Resources&&) = default;
    DX11Resources(DX11Resources const&) = delete;
    DX11Resources& operator=(DX11Resources const&) = delete;
    ~DX11Resources();

    void Reconstruct(UINT flags = c_flip_present, D3D_FEATURE_LEVEL minFeatureLevel = D3D_FEATURE_LEVEL_10_0);
    void ReconstructFactory(UINT flags = c_flip_present);

    UINT GetDeviceOptions() const noexcept { return device_options; }
    D3D_FEATURE_LEVEL GetFeatureLevel() const noexcept { return feature_level; }
    auto GetFactoryPtr() const { return fac.Get(); }
    auto GetDevicePtr() const { return dev.Get(); }
    auto GetContextPtr() const { return ctx.Get(); }
    auto GetAnnotationPtr() const { return anno.Get(); }

    // This is a helper hack to facilitate the refactoring of the DX11Pipeline object, to be removed... 
    Resources GetParams()
    {
        return Resources{
            device_options,
            fac.Get(),
            dev.Get(),
            ctx.Get(),
            anno.Get(),
            this
        };
    }
};
using DX11ResourcesPtr = std::unique_ptr<DX11Resources>;


/// <summary>
/// DX11Pipeline class provides swap chain and buffers
/// </summary>
using SwapChainPtr = Microsoft::WRL::ComPtr<IDXGISwapChain1>;
using RenderTargetTexturePtr = Microsoft::WRL::ComPtr<ID3D11Texture2D>;
using DepthStencilTexturePtr = Microsoft::WRL::ComPtr<ID3D11Texture2D>;
using RenderTargetViewPtr = Microsoft::WRL::ComPtr<ID3D11RenderTargetView>;
using DepthStencilViewPtr = Microsoft::WRL::ComPtr<ID3D11DepthStencilView>;

class DX11Pipeline
{
    bool device_lost = default_device_lost;

    HWND window_handle = nullptr;

    Resources res;

    SwapChainPtr swapchain;
    RenderTargetTexturePtr rendertarget;
    DepthStencilTexturePtr depthstencil;
    RenderTargetViewPtr rtv;
    DepthStencilViewPtr dsv;

    RECT output_rect{};
    D3D11_VIEWPORT viewport{};

    DXGI_FORMAT swapchain_format{ DXGI_FORMAT_UNKNOWN };
    DXGI_FORMAT rendertarget_format{ DXGI_FORMAT_B8G8R8A8_UNORM }; // 4-Byte texture
    DXGI_FORMAT depthstencil_format{ DXGI_FORMAT_D32_FLOAT }; // Float UV
    UINT buffer_count = 2; // Currently hard coded for the texture and UV layers only above

    DXGI_COLOR_SPACE_TYPE colorspace{ DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709 }; // Something to do with HDR

    bool _Is_display_hdr10(); // Checks for HDR feature support on the device

    void _Create_swap_chain(UINT width, UINT height, DXGI_FORMAT format, UINT count);
    void _Create_render_target_view();
    void _Create_depth_stencil_view(UINT width, UINT height);

public:
    DX11Pipeline(HWND hWnd, Resources resources, UINT height, UINT width);
    DX11Pipeline(DX11Pipeline&&) = default;
    DX11Pipeline& operator=(DX11Pipeline&&) = default;
    DX11Pipeline(DX11Pipeline const&) = delete;
    DX11Pipeline& operator=(DX11Pipeline const&) = delete;
    ~DX11Pipeline();

    bool IsDeviceLost() { return device_lost; }
    void SetDeviceLost() { device_lost = true; }
    void Reconstruct(Resources resources, UINT width, UINT height);

    void UpdateColorSpace();
    void ResizeSwapChain(UINT width, UINT height);
    bool WindowResized(UINT width, UINT height);
    void SetViewport(float width, float height);

    void Present(); // Swap the... swapchain

    IDXGISwapChain1* GetSwapChainPtr() { return swapchain.Get(); }
    ID3D11Texture2D* GetRenderTargetPtr() { return rendertarget.Get(); }
    ID3D11Texture2D* GetDepthStencilPtr() { return depthstencil.Get(); }
    ID3D11RenderTargetView* GetRTVPtr() { return rtv.Get(); }
    ID3D11DepthStencilView* GetDSVPtr() { return dsv.Get(); }

    DXGI_FORMAT GetSwapChainFmt() { return swapchain_format; }
    DXGI_FORMAT GetRTVFmt() { return rendertarget_format; }
    DXGI_FORMAT GetDSVFmt() { return depthstencil_format; }
    UINT GetBufferCount() { return buffer_count; }
    RECT GetOutputRect() { return output_rect; }
    D3D11_VIEWPORT GetViewport() { return viewport; }
    DXGI_COLOR_SPACE_TYPE GetColorSpace() { return colorspace; }
};
using DX11PipelinePtr = std::unique_ptr<DX11Pipeline>;


/// <summary>
/// RenderSystem is the primary API for the DX11 Resources and Pipeline
/// The EngineCore uses the IDeviceCallback to interface with the content system to destroy and create content resources.
/// </summary>
interface IDeviceCallback
{
    virtual void OnDeviceLost() = 0;
    virtual void OnDeviceRestored() = 0;

protected:
    ~IDeviceCallback() = default;
};

class RenderSystem
{
    std::mutex m;
    std::thread t;
    std::promise<void> p{};

    std::shared_ptr<ClayEngine::WindowClass> wnd;

    std::unique_ptr<ClayEngine::DX11Resources> res;
    std::unique_ptr<ClayEngine::DX11Pipeline> pipe;

    // The IDeviceNotify can be held directly as it owns the DeviceResources.
    IDeviceCallback* i_device_callback;

public:
    RenderSystem(WindowClassPtr wndPtr) noexcept(true);
    RenderSystem(RenderSystem&&) = default;
    RenderSystem& operator=(RenderSystem&&) = default;
    RenderSystem(RenderSystem const&) = delete;
    RenderSystem& operator=(RenderSystem const&) = delete;
    ~RenderSystem();

    void ResetRenderSystem();
    bool WindowResized(UINT width, UINT height);
    bool IsDeviceLost() { return pipe->IsDeviceLost(); }

    void RegisterDeviceCallback(IDeviceCallback* deviceNotify) noexcept { i_device_callback = deviceNotify; }

    void Present();

    auto GetDevicePtr() const { return res->GetDevicePtr(); }
    auto GetContextPtr() const { return res->GetContextPtr(); }
    auto GetOutputRect() const noexcept { return pipe->GetOutputRect(); }
    auto GetViewport() const noexcept { return pipe->GetViewport(); }
    auto GetRenderTarget() const { return pipe->GetRenderTargetPtr(); }
    auto GetDepthStencil() const { return pipe->GetDepthStencilPtr(); }
    auto GetRTVPtr() const { return pipe->GetRTVPtr(); }
    auto GetDSVPtr() const { return pipe->GetDSVPtr(); }
    auto GetColorSpace() const noexcept { return pipe->GetColorSpace(); }

    // Performance events
    void PIXBeginEvent(_In_z_ const wchar_t* name);
    void PIXEndEvent();
    void PIXSetMarker(_In_z_ const wchar_t* name);
};
using RenderSystemPtr = std::unique_ptr<RenderSystem>;

struct RenderSystemSentinel
{
    void operator()(std::future<void> future, ClayEngine::RenderSystem* res);
private:
    bool last_device_lost = default_device_lost;
};
_CE_END