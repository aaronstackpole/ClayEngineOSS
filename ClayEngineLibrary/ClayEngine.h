#pragma once
/******************************************************************************/
/*                                                                            */
/* ClayEngine Common Header (C) 2022 Epoch Meridian, LLC.                     */
/*                                                                            */
/*                                                                            */
/******************************************************************************/

#include "Strings.h" // String Functions
#include "Services.h" // Shared Services
#include "Platform.h" // Startup/Shutdown
#include "Storage.h" // File System
#include "Settings.h" // Game Settings
#include "Random.h" // Random Functions
#include "Extensions.h" // Interface and Extension Library

namespace ClayEngine
{
	using Future = std::future<void>;
	using Promise = std::promise<void>;
	using Thread = std::thread;
	using Mutex = std::mutex;

	constexpr auto c_pi = 3.1415926535F;
	constexpr auto c_2pi = 6.283185307F;

	constexpr auto c_max_stringbuffer_length = 1024LL;
	constexpr auto c_max_scrollback_length = 20LL;
	constexpr auto c_max_displaybuffer_length = 256LL;
	constexpr auto c_max_stringarray_length = 2048LL;

	constexpr auto c_target_frame_rate = 60UL;
	constexpr auto c_ticks_per_second = 100'000'000ULL;

	using TimePoint = std::chrono::steady_clock::time_point;
	using TimeSpan = std::chrono::steady_clock::duration;
	using Clock = std::chrono::steady_clock;
	//using TimePoint = std::chrono::high_resolution_clock::time_point;
	//using TimeSpan = std::chrono::high_resolution_clock::duration;
	//using Clock = std::chrono::high_resolution_clock;

	using Seconds = std::chrono::seconds;
	using Milliseconds = std::chrono::milliseconds; // 1'000
	using Microseconds = std::chrono::microseconds; // 1'000'000
	using Nanoseconds = std::chrono::nanoseconds; // 1'000'000'000

	using UpdateCallback = std::function<void(float)>;
	using UpdateCallbacks = std::vector<UpdateCallback>;

	using DrawCallback = std::function<void()>;
	using DrawCallbacks = std::vector<DrawCallback>;
}
