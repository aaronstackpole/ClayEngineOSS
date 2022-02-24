#pragma once
/******************************************************************************/
/*                                                                            */
/* ClayEngine Static Services Class Library (C) 2022 Epoch Meridian, LLC.     */
/*                                                                            */
/*                                                                            */
/******************************************************************************/

#include <functional>
#include <vector>
#include <typeinfo>
#include <typeindex>
#include <map>
#include <sstream>
#include <ios>
#include <exception>
#include <memory>
#include <utility>

#include "Strings.h"

namespace ClayEngine
{
	using Function = std::function<void()>;
	using Functions = std::vector<Function>;

	/// <summary>
	/// Stores and provides raw pointers to services created by this Service, but owned by other objects
	/// </summary>
	class Services
	{
		using Type = std::type_index;
		using Object = void*;
		using ServicesMap = std::map<Type, Object>;

		inline static ServicesMap m_services = {};

	public:
		/// <summary>
		/// This factory method will create a unique_ptr object and return it to the caller.
		/// This class will retain a copy of the pointer, and others can request a raw 
		/// poitner to the object, but the objects themselves are expected to be RAII and not
		/// directly owned here, this class just provides pointers. The caller is expected to
		/// validate if the pointer is valid before using it.
		/// </summary>
		template<typename T, typename... Args>
		static std::unique_ptr<T> MakeService(Args&&... args)
		{
			auto it = m_services.find(Type(typeid(T)));
			if (it == m_services.end())
			{
				auto p = std::make_unique<T>(std::forward<Args>(args)...);
				auto o = reinterpret_cast<Object>(p.get());
				auto t = Type(typeid(T));

				std::stringstream ss;
				ss << "MakeService SUCCESS: " << t.name() << " 0x" << std::hex << o;
				WriteLine(ss.str());

				m_services.emplace(t, o);
				return std::move(p);
			}

			throw std::exception("Service not created due to unique key constraint violation");
		}

		/// <summary>
		/// Returns a raw pointer to a unique_ptr object owned by some other class. You are 
		/// expected to check if the poitner is valid before you use it.
		/// </summary>
		template<typename T>
		static T* GetService()
		{
			auto it = m_services.find(Type(typeid(T)));
			if (it != m_services.end())
			{
				auto p = reinterpret_cast<T*>(it->second);
				return p;
			}

			throw std::exception("Service not found");
		}

		/// <summary>
		/// Used to remove a service from the collection that has been deleted. At this time
		/// there's no checking on a pointer from this perspective, so we have to remove the 
		/// service pointer here if we are, for example, resetting some service component
		/// </summary>
		template<typename T>
		static void RemoveService()
		{
			auto it = m_services.find(std::type_index(typeid(T)));
			if (it != m_services.end())
			{
				auto t = Type(typeid(T));

				std::stringstream ss;
				ss << "RemoveService SUCCESS: " << t.name();
				WriteLine(ss.str());

				m_services.erase(it);
				return;
			}
			WriteLine("RemoveService DEBUG: Service not found for removal.");
		}

		/// <summary>
		/// Untested, probably doesn't work
		/// </summary>
		template<typename T>
		static const T& GetServiceRef()
		{
			auto it = m_services.find(Type(typeid(T)));
			if (it != m_services.end())
			{
				auto p = reinterpret_cast<T*>(it->second);
				return *p;
			}

			throw std::exception("Service not found");
		}
	};
}
