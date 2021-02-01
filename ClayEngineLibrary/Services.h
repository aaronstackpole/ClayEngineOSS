#pragma once

#include "ClayEngine.h"

#include <map>
using ServiceMap = std::map<String, uint64_t>;

_CE_BEGIN
struct Services
{
	static void AddService(String key, uint64_t addr);

	template<typename T, class... Args>
	static std::unique_ptr<T> MakeService(String key, Args&&... args)
	{
		auto it = services.find(key);
		if (it == services.end())
		{
			auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
			auto addr = reinterpret_cast<uint64_t>(ptr.get());

			std::wstringstream ss;
			ss << L"Adding Service by Key: " << key << L" Addr: 0x" << std::setfill(L'0') << std::setw(12) << std::hex << addr;
			WriteLine(ss.str());

			services.emplace(key, addr);
			return ptr;
		}

		throw std::exception("Service not created due to unique key constraint violation");
	}


	template<typename T>
	static T* GetService(String key)
	{
		auto it = services.find(key);
		if (it != services.end())
		{
			auto ptr = reinterpret_cast<T*>(it->second);
			return ptr;
		}

		throw std::exception("Service not found");
	}

	static void RemoveService(String key);

private:
	inline static ServiceMap services{};
};
using ServicesPtr = std::unique_ptr<Services>;
_CE_END
