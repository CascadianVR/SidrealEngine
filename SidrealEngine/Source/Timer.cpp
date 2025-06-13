// Timer.cpp
#include "Timer.h"

std::unordered_map<std::string, double> TimerRegistry::timings;
std::mutex TimerRegistry::mutex;

void TimerRegistry::AddTime(const std::string& name, double ms) {
    std::lock_guard<std::mutex> lock(mutex);
    timings[name] += ms;
}

std::unordered_map<std::string, double> TimerRegistry::GetTimings() {
	return timings;
}

void TimerRegistry::Reset() {
    std::lock_guard<std::mutex> lock(mutex);
    timings.clear();
}

ScopedTimer::ScopedTimer(const std::string& name)
    : name(name), startTime(std::chrono::high_resolution_clock::now()) {
}

ScopedTimer::~ScopedTimer() {
    auto endTime = std::chrono::high_resolution_clock::now();
    double elapsedMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    TimerRegistry::AddTime(name, elapsedMs);
}