#include "pch.h"
#include "DX11Resources.h"

using Microsoft::WRL::ComPtr;

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
        std::wcout << L"WARNING: Direct3D Debug Device is not available" << std::endl;
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
            swprintf_s(buff, L"Direct3D Adapter (%u): VID:%04X, PID:%04X - %ls", adapterIndex, desc.VendorId, desc.DeviceId, desc.Description);
            std::wcout << buff << std::endl;

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
            swprintf_s(buff, L"Direct3D Adapter (%u): VID:%04X, PID:%04X - %ls", adapterIndex, desc.VendorId, desc.DeviceId, desc.Description);
            std::wcout << buff << std::endl;

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
            std::wcout << L"Direct3D Adapter - WARP" << std::endl;
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
