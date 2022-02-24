#include "pch.h"

#include "ClayEngine.h"
#include "ServerCoreSystem.h"
#include "NetworkSystem.h"

using namespace ClayEngine;

namespace
{
	ServerCoreSystemPtr g_core = nullptr;
	SettingsPtr g_settings = nullptr;
}

int wmain()
{
	try
	{
		g_settings = std::make_unique<Settings>();
	}
	catch (std::exception ex)
	{
		std::cout << ex.what();
		return -1;
	}

	g_core = Services::MakeService<ServerCoreSystem>();

	while (g_core)
	{
		if (g_core->GetStateChanged())
		{
			g_core->OnStateChanged();

			if (g_core->GetShutdown())
			{
				g_core.reset();
				g_core = nullptr;
			}
		}
	}

	return 0;
}
