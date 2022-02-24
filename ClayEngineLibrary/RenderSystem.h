#pragma once
/******************************************************************************/
/*                                                                            */
/* ClayEngine Rendering System API Class (C) 2022 Epoch Meridian, LLC.        */
/*                                                                            */
/*                                                                            */
/******************************************************************************/

#include "ClayEngine.h"

#include "DX11Resources.h"
#include "DX11PrimitivePipeline.h"

#include "SpriteBatch.h"

namespace ClayEngine
{
	namespace Graphics
	{
		using namespace DirectX;

		using SpriteBatchPtr = std::unique_ptr<SpriteBatch>;
		using SpriteBatchRaw = SpriteBatch*;

		/// <summary>
		/// API entry point for the graphical pipeline and GPU resources
		/// </summary>
		class RenderSystem
		{
			DX11ResourcesPtr m_resources = nullptr;

			SpriteBatchPtr m_spritebatch = nullptr;
			PrimitivePipelinePtr m_primitive = nullptr;

			bool m_device_lost = true;

		public:
			RenderSystem();
			~RenderSystem();

			void StartRenderSystem();
			void StopRenderSystem();
			void RestartRenderSystem();

			void Clear();
			void Present();

			void OnDeviceLost();

			DX11ResourcesRaw GetDeviceResources();

			SpriteBatchRaw GetSpriteBatch();
			PrimitivePipelineRaw GetPrimitivePipeline();
		};
		using RenderSystemPtr = std::unique_ptr<RenderSystem>;
		using RenderSystemRaw = RenderSystem*;
	}
}
