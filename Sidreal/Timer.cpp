#include <time.h>
#include <vector>
#include <unordered_map>
#include <chrono>
#include "Timer.h"
#include <iostream>

std::unordered_map<const char*, Timer::Timer> timerMap;

void Timer::Start(const char* timerName)
{
	if (timerMap.find(timerName) != timerMap.end())
	{
		Timer timer;
		timer.name = timerName;
		timer.start = std::chrono::high_resolution_clock::now();
		timerMap[timerName] = timer;
	}
	else
	{
		Timer timer;
		timer.name = timerName;
		timer.start = std::chrono::high_resolution_clock::now();
		timerMap.insert({ timerName, timer });
	}
}

void Timer::Stop(const char* timerName)
{
	if (timerMap.find(timerName) != timerMap.end())
	{
		Timer timer = timerMap[timerName];

		timer.end = std::chrono::high_resolution_clock::now();
		timer.elapsedTimeMs = std::chrono::duration_cast<std::chrono::microseconds>(timer.end - timer.start).count() / 1000.0f;

		timerMap[timerName] = timer;
	}
	else
	{
		std::cout << "Timer: " << timerName << " not found" << std::endl;
	}
}

std::unordered_map<const char*, Timer::Timer>* Timer::GetTimerMap()
{
	return &timerMap;
}
