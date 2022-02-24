#pragma once
/******************************************************************************/
/*                                                                            */
/* ClayEngine Client Core Class Librar (C) 2022 Epoch Meridian, LLC.          */
/*                                                                            */
/*                                                                            */
/******************************************************************************/


#include "ClayEngine.h"
#include "TimingSystem.h"
#include "RenderSystem.h"
#include "ContentSystem.h"
#include "NetworkSystem.h"

//#include "Sensorium.h"

#include "SquareChase.h"

namespace ClayEngine
{
	using namespace ClayEngine;
	using namespace ClayEngine::Game;
	using namespace ClayEngine::Graphics;
	using namespace ClayEngine::Platform;
	using namespace ClayEngine::Networking;

	enum class ClientCoreState
	{
		Default,
		Initializing,
		DebugRunning,
	};

	/// <summary>
	/// This is the core application state handler responsible for informing various
	/// system modules of what they should be doing at any given time. We also handle
	/// the logic for when and how to shift from one state to the next here.
	/// 
	/// Note this will not dictate the scene that the engine is running, that state
	/// will be managed separately from the application state. This is strictly
	/// for managing the state of the application system modules.
	/// </summary>
	class ClientCoreSystem
	{
		ClientCoreState m_state = ClientCoreState::Default;
		bool m_state_changed = true; // This flag effectively acts as a state gate to stop the MSG loop pump.

		TimingSystemPtr m_timer = nullptr;
		RenderSystemPtr m_render = nullptr;
		ContentSystemPtr m_content = nullptr;
		NetworkSystemPtr m_network = nullptr;

		//SensoriumPtr m_sensorium = nullptr;
		SquareChasePtr m_chase = nullptr;

	public:
		ClientCoreSystem();
		~ClientCoreSystem();

		void StopServices();

		void SetState(ClientCoreState state);

		bool GetStateChanged();

		void OnStateChanged();
	};
	using ClientCoreSystemPtr = std::unique_ptr<ClientCoreSystem>;
	using ClientCoreSystemRaw = ClientCoreSystem*;
}