#pragma once

#include <chrono>
#include <string>
#include <unordered_map>
#include <mutex>
#include <iostream>

class TimerRegistry {
public:
    static void AddTime(const std::string& name, double ms);
    static std::unordered_map<std::string, double> GetTimings();
    static void Reset();

private:
    static std::unordered_map<std::string, double> timings;
    static std::mutex mutex;
};

class ScopedTimer {
public:
    ScopedTimer(const std::string& name);
    ~ScopedTimer();

private:
    std::string name;
    std::chrono::high_resolution_clock::time_point startTime;
};