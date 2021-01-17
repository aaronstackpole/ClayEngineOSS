#pragma once

#include "ClayEngine.h"

_CE_BEGIN
using FactoryPtr = Microsoft::WRL::ComPtr<IDXGIFactory2>;
using DevicePtr = Microsoft::WRL::ComPtr<ID3D11Device1>;
using ContextPtr = Microsoft::WRL::ComPtr<ID3D11DeviceContext1>;
using AnnoPtr = Microsoft::WRL::ComPtr<ID3DUserDefinedAnnotation>;

static const UINT c_flip_present    = 0x01;
static const UINT c_allow_tearing   = 0x02;
static const UINT c_enable_hdr      = 0x04;

static const D3D_FEATURE_LEVEL s_feature_levels[] = {
    D3D_FEATURE_LEVEL_11_1,
    D3D_FEATURE_LEVEL_11_0,
    D3D_FEATURE_LEVEL_10_1,
    D3D_FEATURE_LEVEL_10_0,
    D3D_FEATURE_LEVEL_9_3,
    D3D_FEATURE_LEVEL_9_2,
    D3D_FEATURE_LEVEL_9_1
};

// This is a helper hack to facilitate ongoing refactoring
struct Resources
{
    UINT DeviceOptions;
    IDXGIFactory2* DXGIFactoryPtr;
    ID3D11Device1* D3D11DevicePtr;
    ID3D11DeviceContext1* D3D11ContextPtr;
    ID3DUserDefinedAnnotation* D3DAnnotationPtr;
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
    DX11Resources& operator= (DX11Resources&&) = default;
    DX11Resources(DX11Resources const&) = delete;
    DX11Resources& operator= (DX11Resources const&) = delete;
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
            anno.Get()
        };
    }
};
using DX11ResourcesPtr = std::unique_ptr<DX11Resources>;
_CE_END
