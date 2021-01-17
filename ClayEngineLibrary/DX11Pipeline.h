#pragma once

#include "DX11Resources.h"

_CE_BEGIN
using SwapChainPtr = Microsoft::WRL::ComPtr<IDXGISwapChain1>;
using RenderTargetTexturePtr = Microsoft::WRL::ComPtr<ID3D11Texture2D>;
using DepthStencilTexturePtr = Microsoft::WRL::ComPtr<ID3D11Texture2D>;
using RenderTargetViewPtr = Microsoft::WRL::ComPtr<ID3D11RenderTargetView>;
using DepthStencilViewPtr = Microsoft::WRL::ComPtr<ID3D11DepthStencilView>;

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
	~DX11Pipeline();

	bool IsDeviceLost() { return device_lost; }
	void SetDeviceLost() { device_lost = true; }
	void Reconstruct(Resources resources, UINT width, UINT height);

	bool WindowSizeChanged(UINT width, UINT height);
	void ResizeSwapChain(UINT width, UINT height);
	void UpdateColorSpace();
	void SetViewport(float width, float height);

	void Present(); // Swap the... swapchain

	auto GetSwapChainPtr() { return swapchain.Get(); }
	auto GetRenderTargetPtr() { return rendertarget.Get(); }
	auto GetDepthStencilPtr() { return depthstencil.Get(); }
	auto GetRTVPtr() { return rtv.Get(); }
	auto GetDSVPtr() { return dsv.Get(); }

	DXGI_FORMAT GetSwapChainFmt() { return swapchain_format; }
	DXGI_FORMAT GetRTVFmt() { return rendertarget_format; }
	DXGI_FORMAT GetDSVFmt() { return depthstencil_format; }
	UINT GetBufferCount() { return buffer_count; }
	RECT GetOutputRect() { return output_rect; }
	D3D11_VIEWPORT GetViewport() { return viewport; }
	DXGI_COLOR_SPACE_TYPE GetColorSpace() { return colorspace; }
};
using DX11PipelinePtr = std::unique_ptr<DX11Pipeline>;
_CE_END