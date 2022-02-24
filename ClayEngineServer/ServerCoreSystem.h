#pragma once
/******************************************************************************/
/*                                                                            */
/* ClayEngine Server Core Class Library (C) 2022 Epoch Meridian, LLC.         */
/*                                                                            */
/*                                                                            */
/******************************************************************************/

#include "ClayEngine.h"
#include "NetworkSystem.h"

namespace ClayEngine
{
	using namespace ClayEngine;
	using namespace ClayEngine::Platform;
	using namespace ClayEngine::Networking;

	enum class ServerCoreState
	{
		Default,
		Initializing,
		DebugRunning,
		Shutdown,
	};

	class ServerCoreSystem
	{
		ServerCoreState m_state = ServerCoreState::Default;
		bool m_state_changed = true;
		bool m_shutdown = false;

		NetworkSystemPtr m_network = nullptr;

	public:
		ServerCoreSystem()
		{

		}
		~ServerCoreSystem()
		{
			StopServices();
		}

		void StopServices()
		{
			if (m_network)
			{
				Services::RemoveService<NetworkSystem>();

				m_network.reset();
				m_network = nullptr;
			}
		}

		void SetState(ServerCoreState state)
		{
			m_state_changed = true;
			m_state = state;
		}

		bool GetShutdown()
		{
			return m_shutdown;
		}

		bool GetStateChanged()
		{
			return m_state_changed;
		}

		void OnStateChanged()
		{
			m_state_changed = false;
			switch (m_state)
			{
			case ServerCoreState::Default:
				{
					WriteLine("OnStateChanged INFO: ServerCoreState::Default");

					m_state = ServerCoreState::Initializing;
					m_state_changed = true;
				}
				break;
			case ServerCoreState::Initializing:
				{
					WriteLine("OnStateChanged INFO: ServerCoreState::Initializing");

					m_network = Services::MakeService<NetworkSystem>();
					m_network->SetListenServerHints(AF_INET, SOCK_STREAM, IPPROTO_TCP);
					m_network->SetListenServerPort(48000);
					m_network->SetListenServerTimeout(125);
					m_network->StartListenServer();

					m_state = ServerCoreState::DebugRunning;
					m_state_changed = true;
				}
				break;
			case ServerCoreState::DebugRunning:
				{
					WriteLine("OnStateChanged INFO: ServerCoreState::DebugRunning");

					m_network->Run();

					m_state = ServerCoreState::Shutdown;
					m_state_changed = true;
				}
				break;
			case ServerCoreState::Shutdown:
				{
					WriteLine("OnStateChanged INFO: ServerCoreState::Shutdown");

					StopServices();

					m_shutdown = true;
				}
				break;
			default:
				break;
			}

		}
	};
	using ServerCoreSystemPtr = std::unique_ptr<ServerCoreSystem>;
	using ServerCoreSystemRaw = ServerCoreSystem*;
}
