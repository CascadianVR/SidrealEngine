#pragma once
#include <unordered_map>
#include <chrono>

namespace Timer
{
	struct Timer
	{
		const char* name;
		std::chrono::time_point<std::chrono::steady_clock> start, end;
		float elapsedTimeMs;
	};

	void Start(const char* timerName);
	void Stop(const char* timerName);

	std::unordered_map<const char*, Timer>* GetTimerMap();
}