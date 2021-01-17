#include "pch.h"
#include "DX11Pipeline.h"

using Microsoft::WRL::ComPtr;

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

bool ClayEngine::DX11Pipeline::WindowSizeChanged(UINT width, UINT height)
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
        //HandleDeviceLost();
        // - Notify callback of device lost
        // - Delete pipeline and this
        // - Create this and pipeline
        // - Notify callback of device restored
    }
    else
    {
        ThrowIfFailed(hr);

        if (!res.DXGIFactoryPtr->IsCurrent())
        {
            // Output information is cached on the DXGI Factory. If it is stale we need to create a new factory.
            //CreateFactory();
        }
    }
}
