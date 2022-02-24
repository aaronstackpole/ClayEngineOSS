#include "pch.h"
#include "ClientCoreSystem.h"

using namespace ClayEngine;

ClientCoreSystem::ClientCoreSystem()
{
	m_timer = Services::MakeService<TimingSystem>();
}

ClientCoreSystem::~ClientCoreSystem()
{
	StopServices();
}

void ClientCoreSystem::StopServices()
{
	m_timer->StopTimer();

	if (m_chase)
	{
		m_chase.reset();
		m_chase = nullptr;
	}

	//if (m_sensorium)
	//{
	//	m_sensorium.reset();
	//	m_sensorium = nullptr;
	//}

	if (m_network)
	{
		m_network->StopClientConnection();
		m_network.reset();
		m_network = nullptr;
	}

	if (m_content)
	{
		m_content.reset();
		m_content = nullptr;
	}

	if (m_render)
	{
		m_render.reset();
		m_render = nullptr;
	}

	if (m_timer)
	{
		m_timer.reset();
		m_timer = nullptr;
	}
}

void ClientCoreSystem::SetState(ClientCoreState state)
{
	m_state_changed = true;
	m_state = state;
}

bool ClientCoreSystem::GetStateChanged()
{
	return m_state_changed;
}

void ClientCoreSystem::OnStateChanged()
{
	m_state_changed = false;
	switch (m_state)
	{
	case ClientCoreState::Default: // The platform has been initialized, begin loading rendering and content systems
		{
			WriteLine("OnStateChanged INFO: ClientCoreState::Default");

			// Initialize the API for the rendering system. This class creates and destroys resources related
			// to the graphics device, and provides graphics related objects for use by the engine.
			m_render = Services::MakeService<RenderSystem>();

			// In the future, this is meant to be templatized and used by defining which rendering system to use.
			// Currently it just loads simple DX11 modules
			m_render->StartRenderSystem();

			// Initialize the API for the content system. This class will load resources for rendering from disk.
			m_content = Services::MakeService<ContentSystem>();


			// Shift to the next state. Currently this doesn't do anything because we don't call OnStateChanged again at this time.
			m_state = ClientCoreState::Initializing;
			m_state_changed = true;
		}
		break;
	case ClientCoreState::Initializing:
		{
			WriteLine("OnStateChanged INFO: ClientCoreState::Initializing");

			// Initialize the user interface root class and hook it up to the ticker
			//m_sensorium = Services::MakeService<Sensorium>();
			//m_timer->AddUpdateCallback([&](float elapsedTime) { m_sensorium->Update(elapsedTime); });

			m_state = ClientCoreState::DebugRunning;
			m_state_changed = true;
		}
		break;
	case ClientCoreState::DebugRunning:
		{
			WriteLine("OnStateChanged INFO: ClientCoreState::DebugRunning");

			m_chase = std::make_unique<SquareChase>();
			m_timer->AddUpdateCallback([&](float elapsedTime) { m_chase->Update(elapsedTime); });

			// The ticker was initialized during the startup of this object, it runs in its own thread, we tell it
			// to start pumping ticks (Update/Draw) to the game engine.
			m_timer->StartTimer();

			// Temporary code testing network client connection. Server has to be running for this to work.
			m_network = Services::MakeService<ClayEngine::Networking::NetworkSystem>();
			m_network->SetClientConnectionHints(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			m_network->SetClientConnectionPort(48000);
			//m_network->StartClientConnection("73.210.118.242");
		}
		break;
	default:
		break;
	}
}
