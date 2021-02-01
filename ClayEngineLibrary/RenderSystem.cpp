#include "pch.h"
#include "RenderSystem.h"
#include "Services.h"

#pragma warning(disable : 4061)

using namespace DirectX;
using namespace ClayEngine;

using Microsoft::WRL::ComPtr;

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

#pragma region ClayEngine::DX11Resources
#ifdef _DEBUG
bool ClayEngine::DX11Resources::_Is_debug_device_available() noexcept
{
    HRESULT hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_NULL,       // There is no need to create a real hardware device.
        nullptr,
        D3D11_CREATE_DEVICE_DEBUG,  // Check for the SDK layers.
        nullptr,                    // Any feature level will do.
        0,
        D3D11_SDK_VERSION,
        nullptr,                    // No need to keep the D3D device reference.
        nullptr,                    // No need to know the feature level.
        nullptr                     // No need to keep the D3D device context reference.
    );

    return SUCCEEDED(hr);
}
#endif
void ClayEngine::DX11Resources::_Create_factory_and_flags()
{
#ifdef _DEBUG
    if (_Is_debug_device_available())
    {
        // If the project is in a debug build, enable debugging via SDK Layers with this flag.
        creation_flags |= D3D11_CREATE_DEVICE_DEBUG;
    }
    else
    {
        WriteLine(L"WARNING: Direct3D Debug Device is not available");
    }
#endif

#ifdef _DEBUG
    Microsoft::WRL::ComPtr<IDXGIInfoQueue> dxgi_queue;
    if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgi_queue.GetAddressOf()))))
    {
        ThrowIfFailed(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(fac.ReleaseAndGetAddressOf())));

        dxgi_queue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
        dxgi_queue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);

        DXGI_INFO_QUEUE_MESSAGE_ID hide[] =
        {
            80 /* IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides. */,
        };
        DXGI_INFO_QUEUE_FILTER filter = {};
        filter.DenyList.NumIDs = _countof(hide);
        filter.DenyList.pIDList = hide;
        dxgi_queue->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);
    }
    else
    {
#endif
        ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(fac.ReleaseAndGetAddressOf())));
#ifdef _DEBUG
    }
#endif
}

void ClayEngine::DX11Resources::_Validate_device_options()
{
    // Determines whether tearing support is available for fullscreen borderless windows.
    if (device_options & c_allow_tearing)
    {
        BOOL allowTearing = FALSE;

        ComPtr<IDXGIFactory5> factory5;
        HRESULT hr = fac.As(&factory5);
        if (SUCCEEDED(hr))
        {
            hr = factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
        }

        if (FAILED(hr) || !allowTearing)
        {
            device_options &= ~c_allow_tearing;
            std::wcout << L"WARNING: Variable refresh rate displays not supported" << std::endl;
        }
    }

    // Disable HDR if we are on an OS that can't support FLIP swap effects
    if (device_options & c_enable_hdr)
    {
        ComPtr<IDXGIFactory5> factory5;
        if (FAILED(fac.As(&factory5)))
        {
            device_options &= ~c_enable_hdr;
            std::wcout << L"WARNING: HDR swap chains not supported" << std::endl;
        }
    }

    // Disable FLIP if not on a supporting OS
    if (device_options & c_flip_present)
    {
        ComPtr<IDXGIFactory4> factory4;
        if (FAILED(fac.As(&factory4)))
        {
            device_options &= ~c_flip_present;
            std::wcout << L"INFO: Flip swap effects not supported" << std::endl;
        }
    }

    // Determine DirectX hardware feature levels this app will support.
    for (; feature_level_count < _countof(s_feature_levels); ++feature_level_count)
    {
        if (s_feature_levels[feature_level_count] < min_feature_level) break;
    }
    if (!feature_level_count) { throw std::out_of_range("Requested minimum feature level unsupported by device"); }
}

void ClayEngine::DX11Resources::_Create_device_and_context()
{
    ComPtr<IDXGIAdapter1> adapter;
    HRESULT hr;

    // Create the fancy debug device if we can
#if defined(__dxgi1_6_h__) && defined(NTDDI_WIN10_RS4)
    ComPtr<IDXGIFactory6> factory6;
    hr = fac.As(&factory6);
    if (SUCCEEDED(hr))
    {
        for (UINT adapterIndex = 0;
            SUCCEEDED(factory6->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
                IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf())));
            adapterIndex++)
        {
            DXGI_ADAPTER_DESC1 desc;
            ThrowIfFailed(adapter->GetDesc1(&desc));

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) { continue; } // Don't select the Basic Render Driver adapter.

            wchar_t buff[256] = {};
            swprintf_s(buff, L"Using Adapter %u: Name: %ls, Device ID: %04X", adapterIndex, desc.Description, desc.DeviceId);
            WriteLine(buff);

            break;
        }
    }
#endif

    // Otherwise, create the default factory1
    if (!adapter)
    {
        for (UINT adapterIndex = 0;
            SUCCEEDED(fac->EnumAdapters1(adapterIndex,
                adapter.ReleaseAndGetAddressOf()));
            adapterIndex++)
        {
            DXGI_ADAPTER_DESC1 desc;
            ThrowIfFailed(adapter->GetDesc1(&desc));

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) { continue; } // Don't select the Basic Render Driver adapter.

            wchar_t buff[256] = {};
            swprintf_s(buff, L"Using Adapter %u: Name: %ls, Device ID: %04X", adapterIndex, desc.Description, desc.DeviceId);
            WriteLine(buff);

            break;
        }
    }

    // If we didn't create that, something went wrong...
    if (!adapter) throw;

    // Create the Direct3D 11 API device object and a corresponding context.
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;

    hr = E_FAIL;
    if (adapter)
    {
        hr = D3D11CreateDevice(
            adapter.Get(),
            D3D_DRIVER_TYPE_UNKNOWN,
            nullptr,
            creation_flags,
            s_feature_levels,
            feature_level_count,
            D3D11_SDK_VERSION,
            device.GetAddressOf(),  // Returns the Direct3D device created.
            &feature_level,     // Returns feature level of device created.
            context.GetAddressOf()  // Returns the device immediate context.
        );
    }
#ifdef NDEBUG
    else
    {
        throw std::exception("No Direct3D hardware device found");
    }
#else
    if (FAILED(hr))
    {
        // If the initialization fails, fall back to the WARP device.
        // For more information on WARP, see:
        // http://go.microsoft.com/fwlink/?LinkId=286690
        hr = D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_WARP, // Create a WARP device instead of a hardware device.
            nullptr,
            creation_flags,
            s_feature_levels,
            feature_level_count,
            D3D11_SDK_VERSION,
            device.GetAddressOf(),
            &feature_level,
            context.GetAddressOf()
        );

        if (SUCCEEDED(hr))
        {
            WriteLine(L"Direct3D Adapter - WARP");
        }
    }
#endif

    ThrowIfFailed(hr);

#ifndef NDEBUG
    ComPtr<ID3D11Debug> d3dDebug;
    if (SUCCEEDED(device.As(&d3dDebug)))
    {
        ComPtr<ID3D11InfoQueue> d3dInfoQueue;
        if (SUCCEEDED(d3dDebug.As(&d3dInfoQueue)))
        {
#ifdef _DEBUG
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif
            D3D11_MESSAGE_ID hide[] =
            {
                D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
            };
            D3D11_INFO_QUEUE_FILTER filter = {};
            filter.DenyList.NumIDs = _countof(hide);
            filter.DenyList.pIDList = hide;
            d3dInfoQueue->AddStorageFilterEntries(&filter);
        }
    }
#endif

    ThrowIfFailed(device.As(&dev));
    ThrowIfFailed(context.As(&ctx));
    ThrowIfFailed(context.As(&anno));
}

ClayEngine::DX11Resources::DX11Resources(UINT flags, D3D_FEATURE_LEVEL minFeatureLevel)
    : feature_level{ D3D_FEATURE_LEVEL_9_1 }
{
    device_options |= flags;

    _Create_factory_and_flags();
    _Validate_device_options();
    _Create_device_and_context();
}

ClayEngine::DX11Resources::~DX11Resources()
{
    anno.Reset();
    ctx.Reset();

#ifdef _DEBUG
    {
        ComPtr<ID3D11Debug> dbg;
        if (SUCCEEDED(dev.As(&dbg)))
        {
            dbg->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY);
        }
    }
#endif

    dev.Reset();
    fac.Reset();
}

void ClayEngine::DX11Resources::Reconstruct(UINT flags, D3D_FEATURE_LEVEL minFeatureLevel)
{
    anno.Reset();
    ctx.Reset();

#ifdef _DEBUG
    {
        ComPtr<ID3D11Debug> dbg;
        if (SUCCEEDED(dev.As(&dbg)))
        {
            dbg->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY);
        }
    }
#endif

    dev.Reset();
    fac.Reset();

    feature_level = D3D_FEATURE_LEVEL_9_1;
    device_options = 0;
    device_options |= flags;

    _Create_factory_and_flags();
    _Validate_device_options();
    _Create_device_and_context();
}

void ClayEngine::DX11Resources::ReconstructFactory(UINT flags)
{
    fac.Reset();

    device_options = 0;
    device_options |= flags;

    _Create_factory_and_flags();
}
#pragma endregion

#pragma region ClayEngine::DX11Pipeline
bool ClayEngine::DX11Pipeline::_Is_display_hdr10()
{
#if defined(NTDDI_WIN10_RS2)
    if (swapchain)
    {
        ComPtr<IDXGIOutput> output;
        if (SUCCEEDED(swapchain->GetContainingOutput(output.GetAddressOf())))
        {
            ComPtr<IDXGIOutput6> output6;
            if (SUCCEEDED(output.As(&output6)))
            {
                DXGI_OUTPUT_DESC1 desc;
                ThrowIfFailed(output6->GetDesc1(&desc));

                if (desc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020)
                {
                    // Display output is HDR10.
                    return true;
                }
            }
        }
    }
#endif
    return false;
}

void ClayEngine::DX11Pipeline::_Create_swap_chain(UINT width, UINT height, DXGI_FORMAT format, UINT count)
{
    // Create a descriptor for the swap chain.
    DXGI_SWAP_CHAIN_DESC1 desc{};
    desc.Width = width;
    desc.Height = height;
    desc.Format = format;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = count;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Scaling = DXGI_SCALING_STRETCH;
    desc.SwapEffect = (res.DeviceOptions & (c_flip_present | c_allow_tearing | c_enable_hdr)) ? DXGI_SWAP_EFFECT_FLIP_DISCARD : DXGI_SWAP_EFFECT_DISCARD;
    desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
    desc.Flags = (res.DeviceOptions & c_allow_tearing) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u;

    DXGI_SWAP_CHAIN_FULLSCREEN_DESC fs_desc{};
    fs_desc.Windowed = TRUE;

    // Create a SwapChain from a Win32 window.
    ThrowIfFailed(res.DXGIFactoryPtr->CreateSwapChainForHwnd(res.D3D11DevicePtr, window_handle, &desc, &fs_desc, nullptr, swapchain.ReleaseAndGetAddressOf()));

    // This class does not support exclusive full-screen mode and prevents DXGI from responding to the ALT+ENTER shortcut
    ThrowIfFailed(res.DXGIFactoryPtr->MakeWindowAssociation(window_handle, DXGI_MWA_NO_ALT_ENTER));

}

void ClayEngine::DX11Pipeline::_Create_render_target_view()
{
    // Theoretically, if this is public, we should check to make sure these pointers are valid
    // before we try to access them, however, this function may become private, so TBD...
    ThrowIfFailed(swapchain->GetBuffer(0, IID_PPV_ARGS(rendertarget.ReleaseAndGetAddressOf())));
    CD3D11_RENDER_TARGET_VIEW_DESC desc(D3D11_RTV_DIMENSION_TEXTURE2D, rendertarget_format);
    ThrowIfFailed(res.D3D11DevicePtr->CreateRenderTargetView(rendertarget.Get(), &desc, rtv.ReleaseAndGetAddressOf()));
}

void ClayEngine::DX11Pipeline::_Create_depth_stencil_view(UINT width, UINT height)
{
    // It appears that we need to allocate this buffer on the device as part of this step, compared
    // to above where it appears the buffer is created when the swapchain is allocated.
    if (depthstencil_format != DXGI_FORMAT_UNKNOWN)
    {
        // Create a depth stencil view for use with 3D rendering if needed.
        CD3D11_TEXTURE2D_DESC tex2d_desc(depthstencil_format, width, height,
            1, // This depth stencil view has only one texture.
            1, // Use a single mipmap level.
            D3D11_BIND_DEPTH_STENCIL);
        ThrowIfFailed(res.D3D11DevicePtr->CreateTexture2D(&tex2d_desc, nullptr, depthstencil.ReleaseAndGetAddressOf()));

        CD3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc(D3D11_DSV_DIMENSION_TEXTURE2D);
        ThrowIfFailed(res.D3D11DevicePtr->CreateDepthStencilView(depthstencil.Get(), &dsv_desc, dsv.ReleaseAndGetAddressOf()));
    }
}

ClayEngine::DX11Pipeline::DX11Pipeline(HWND hWnd, Resources resources, UINT width, UINT height)
    : window_handle{ hWnd }
    , res{ resources }
    , output_rect{ 0L, 0L, static_cast<LONG>(width), static_cast<LONG>(height) }
{
    swapchain_format = (res.DeviceOptions & (c_flip_present | c_allow_tearing | c_enable_hdr)) ? NoSRGB(rendertarget_format) : rendertarget_format;

    _Create_swap_chain(width, height, swapchain_format, 2); //TODO: Extend class for additional buffers as needed
    _Create_render_target_view();
    _Create_depth_stencil_view(width, height);

    UpdateColorSpace();
    SetViewport(1.f, 1.f); //TODO: Extend class to allow viewport size control

    device_lost = false;
}

ClayEngine::DX11Pipeline::~DX11Pipeline()
{
    dsv.Reset();
    depthstencil.Reset();
    rtv.Reset();
    rendertarget.Reset();
    swapchain.Reset();
}

void ClayEngine::DX11Pipeline::UpdateColorSpace()
{
    DXGI_COLOR_SPACE_TYPE cs = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;

    if ((res.DeviceOptions & c_enable_hdr) && _Is_display_hdr10())
    {
        switch (rendertarget_format)
        {
        case DXGI_FORMAT_R10G10B10A2_UNORM:
            // The application creates the HDR10 signal.
            cs = DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
            break;

        case DXGI_FORMAT_R16G16B16A16_FLOAT:
            // The system creates the HDR10 signal; application uses linear values.
            cs = DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709;
            break;

        default:
            break;
        }
    }

    colorspace = cs;

    ComPtr<IDXGISwapChain3> swapchain3;
    if (SUCCEEDED(swapchain.As(&swapchain3)))
    {
        UINT color_space_support = 0;
        if (SUCCEEDED(swapchain3->CheckColorSpaceSupport(cs, &color_space_support))
            && (color_space_support & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT))
        {
            ThrowIfFailed(swapchain3->SetColorSpace1(cs));
        }
    }
}

void ClayEngine::DX11Pipeline::ResizeSwapChain(UINT width, UINT height)
{
    ID3D11RenderTargetView* null_rtv[] = { nullptr };
    res.D3D11ContextPtr->OMSetRenderTargets(_countof(null_rtv), null_rtv, nullptr);

    dsv.Reset();
    depthstencil.Reset();
    rtv.Reset();
    rendertarget.Reset();

    res.D3D11ContextPtr->Flush();

    HRESULT hr = swapchain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN,
        (res.DeviceOptions & c_allow_tearing) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u);

    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
    {
        device_lost = true; // Extremely rare chance of Tick trying to do something before this flag is set
#ifdef _DEBUG
        PrintHR(L"Device lost on ResizeBuffers", static_cast<unsigned int>((hr == DXGI_ERROR_DEVICE_REMOVED) ? res.D3D11DevicePtr->GetDeviceRemovedReason() : hr));
#endif
        return;
    }
    else
    {
        ThrowIfFailed(hr);
    }

    swapchain_format = (res.DeviceOptions & (c_flip_present | c_allow_tearing | c_enable_hdr)) ? NoSRGB(rendertarget_format) : rendertarget_format;

    _Create_swap_chain(width, height, swapchain_format, 2); //TODO: Extend class for additional buffers as needed
    _Create_render_target_view();
    _Create_depth_stencil_view(width, height);

    UpdateColorSpace();
    SetViewport(1.f, 1.f); //TODO: Extend class to allow viewport size control
}

bool ClayEngine::DX11Pipeline::WindowResized(UINT width, UINT height)
{
    RECT new_rec{ 0L, 0L, static_cast<LONG>(width), static_cast<LONG>(height) };
    if (new_rec == output_rect)
    {
        UpdateColorSpace();
        SetViewport(1.f, 1.f); //TODO: Extend class to allow viewport size control
        return false;
    }
    output_rect = new_rec;

    ResizeSwapChain(width, height);
    SetViewport(1.f, 1.f); //TODO: Extend class to allow viewport size control
    return true;
}

void ClayEngine::DX11Pipeline::Reconstruct(Resources resources, UINT width, UINT height)
{
    ID3D11RenderTargetView* null_rtv[] = { nullptr };

    if (!device_lost) { res.D3D11ContextPtr->OMSetRenderTargets(_countof(null_rtv), null_rtv, nullptr); }

    dsv.Reset();
    depthstencil.Reset();
    rtv.Reset();
    rendertarget.Reset();
    swapchain.Reset();

    if (!device_lost) { res.D3D11ContextPtr->Flush(); }

    // Assuming that if we're not in a device lost scenario, we use the existing resources
    if (device_lost) { res = resources; }

    swapchain_format = (res.DeviceOptions & (c_flip_present | c_allow_tearing | c_enable_hdr)) ? NoSRGB(rendertarget_format) : rendertarget_format;

    _Create_swap_chain(width, height, swapchain_format, 2); //TODO: Extend class for additional buffers as needed
    _Create_render_target_view();
    _Create_depth_stencil_view(width, height);

    UpdateColorSpace();
    SetViewport(1.f, 1.f); //TODO: Extend class to allow viewport size control
}

void ClayEngine::DX11Pipeline::SetViewport(float width, float height)
{
    viewport = CD3D11_VIEWPORT{ 0.f, 0.f, width, height };
}

void ClayEngine::DX11Pipeline::Present()
{
    HRESULT hr = E_FAIL;
    if (res.DeviceOptions & c_allow_tearing)
    {
        // Recommended to always use tearing if supported when using a sync interval of 0.
        hr = swapchain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
    }
    else
    {
        // The first argument instructs DXGI to block until VSync, putting the application
        // to sleep until the next VSync. This ensures we don't waste any cycles rendering
        // frames that will never be displayed to the screen.
        hr = swapchain->Present(1, 0);
    }

    // Discard the contents of the render target.
    // This is a valid operation only when the existing contents will be entirely
    // overwritten. If dirty or scroll rects are used, this call should be removed.
    res.D3D11ContextPtr->DiscardView(rtv.Get());

    if (dsv)
    {
        // Discard the contents of the depth stencil.
        res.D3D11ContextPtr->DiscardView(dsv.Get());
    }

    // If the device was removed either by a disconnection or a driver upgrade, we
    // must recreate all device resources.
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
    {
#ifdef _DEBUG
        PrintHR(L"", static_cast<unsigned int>((hr == DXGI_ERROR_DEVICE_REMOVED) ? res.D3D11DevicePtr->GetDeviceRemovedReason() : hr));
#endif
        device_lost = true;
    }
    else
    {
        ThrowIfFailed(hr);

        if (!res.DXGIFactoryPtr->IsCurrent())
        {
            // Output information is cached on the DXGI Factory. If it is stale we need to create a new factory.
            res.DX11ResourcesPtr->ReconstructFactory();
        }
    }
}
#pragma endregion

#pragma region ClayEngine::RenderSystem
ClayEngine::RenderSystem::RenderSystem(WindowClassPtr wndPtr) noexcept
    : wnd{ wndPtr }
    , i_device_callback(nullptr)
{
    res = Services::MakeService<DX11Resources>(L"DX11Resources");
    pipe = Services::MakeService<DX11Pipeline>(L"DX11Pipeline",
        wnd->GetWindowHandle(),
        res->GetParams(),
        wnd->GetWindowWidth(),
        wnd->GetWindowHeight()
        );

    t = std::thread{ RenderSystemSentinel(), std::move(p.get_future()), this };
}

ClayEngine::RenderSystem::~RenderSystem()
{
    p.set_value();
    if (t.joinable()) t.join();

    pipe.reset();
    res.reset();
}

void ClayEngine::RenderSystem::ResetRenderSystem()
{
    m.lock();
    // Calls the EngineCore letting it know to destroy content objects
    if (i_device_callback) { i_device_callback->OnDeviceLost(); }

    pipe.reset();
    res.reset();

    res = std::make_unique<DX11Resources>();
    pipe = std::make_unique<DX11Pipeline>(
        wnd->GetWindowHandle(),
        res->GetParams(),
        wnd->GetWindowWidth(),
        wnd->GetWindowHeight()
        );

    // Calls the EngineCore letting it know to create the content objects
    if (i_device_callback) { i_device_callback->OnDeviceRestored(); }
    m.unlock();
}

// This method is called when the Win32 window changes size
bool ClayEngine::RenderSystem::WindowResized(UINT width, UINT height)
{
    m.lock();
    auto result = pipe->WindowResized(width, height);
    m.unlock();

    return result;
}

// Present the contents of the back buffers to the screen.
void ClayEngine::RenderSystem::Present()
{
    m.lock();
    HRESULT hr = E_FAIL;
    if (res->GetDeviceOptions() & c_allow_tearing)
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

void ClayEngine::RenderSystem::PIXBeginEvent(_In_z_ const wchar_t* name)
{
    res->GetAnnotationPtr()->BeginEvent(name);
}

void ClayEngine::RenderSystem::PIXEndEvent()
{
    res->GetAnnotationPtr()->EndEvent();
}

void ClayEngine::RenderSystem::PIXSetMarker(_In_z_ const wchar_t* name)
{
    res->GetAnnotationPtr()->SetMarker(name);
}

// This functor monitors for device failure flag and then locks the DX11 device resources and rebuilds them
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
#pragma endregion
