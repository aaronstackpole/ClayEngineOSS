#include "pch.h"
#include "Services.h"

void ClayEngine::Services::AddService(String key, uint64_t addr)
{
	auto it = services.find(key);
	if (it == services.end())
	{
		std::wstringstream ss;
		ss << L"Adding Service by Key: " << key << L" Addr: 0x" << std::setfill(L'0') << std::setw(16) << std::hex << addr;
		WriteLine(ss.str());

		services.emplace(key, addr);
		return;
	}

	throw std::exception("Service not added due to unique key constraint violation");
}

void ClayEngine::Services::RemoveService(String key)
{
	auto it = services.find(key);
	if (it != services.end())
	{
		services.erase(it);
	}

	throw std::exception("Service not found for removal");
}
