#pragma once

#include "ClayEngine.h"

#include <chrono>

_CE_BEGIN
/// <summary>
/// Timer class provides interval timing information for Update calls
/// </summary>
using TimePoint = std::chrono::steady_clock::time_point;
using TimeSpan = std::chrono::steady_clock::duration;
using Clock = std::chrono::steady_clock;
using Seconds = std::chrono::seconds;
using Microseconds = std::chrono::microseconds;

static const uint64_t TicksPerSecond = 10'000'000;

static constexpr double TicksToSeconds(uint64_t ticks) noexcept { return static_cast<double>(ticks) / TicksPerSecond; }
static constexpr uint64_t SecondsToTicks(double seconds) noexcept { return static_cast<uint64_t>(seconds * TicksPerSecond); }

class Timer
{
	TimePoint last_time_point;
	TimeSpan frames_time_span;
	TimeSpan total_time_span;
	TimeSpan update_time_span;

	uint32_t frames_per_second = 0;
	uint32_t frames_this_second = 0;

	bool is_fixed_update = false;
	uint32_t target_frame_rate = 60;
	TimeSpan target_update_time = Microseconds(1'000'000 / target_frame_rate);

public:
	Timer()
	{
		total_time_span = TimeSpan::zero();

		ResetElapsedTime();
	}
	~Timer() = default;

	void ResetElapsedTime()
	{
		last_time_point = Clock::now();

		frames_per_second = 0;
		frames_this_second = 0;

		frames_time_span = TimeSpan::zero();
		update_time_span = TimeSpan::zero();
	}

	template <typename T>
	void Tick(const T& update)
	{
		auto now_time_point = Clock::now();
		auto delta_time_span = now_time_point - last_time_point;
		last_time_point = now_time_point;

		total_time_span += delta_time_span;
		update_time_span += delta_time_span;
		frames_time_span += delta_time_span;
		++frames_this_second;

		if (frames_time_span >= Seconds(1))
		{
			frames_per_second = frames_this_second;
			frames_this_second = 0;
			frames_time_span %= Seconds(1);
		}

		if (is_fixed_update)
		{
			if (update_time_span >= target_update_time)
			{
				update_time_span %= target_update_time;
				update();
			}
		}
		else
		{
			update_time_span = TimeSpan::zero();
			update();
		}
	}

	double GetElapsedSeconds() const noexcept { return TicksToSeconds(frames_time_span.count()); }
	double GetTotalSeconds() const noexcept { return TicksToSeconds(total_time_span.count()); }
	uint64_t GetElapsedTicks() const noexcept { return static_cast<uint64_t>(frames_time_span.count()); }
	uint64_t GetTotalTicks() const noexcept { return static_cast<uint64_t>(total_time_span.count()); }

	uint32_t GetFrameCount() const noexcept { return frames_this_second; }
	uint32_t GetFramesPerSecond() const noexcept { return frames_per_second; }

	void SetFixedUpdate(bool isFixed) noexcept { is_fixed_update = isFixed; }
	void SetTargetFramerate(uint32_t framerate) noexcept { target_update_time = Seconds(1 / framerate); }
};
_CE_END
