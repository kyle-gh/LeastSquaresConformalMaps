
#pragma once

#include <chrono>
#include <iostream>

#define TIMER_START(NAME) \
std::chrono::high_resolution_clock::time_point __TIMER##NAME##_START = std::chrono::high_resolution_clock::now();


#define TIMER_END(NAME) \
std::chrono::high_resolution_clock::time_point __TIMER##NAME##_END = std::chrono::high_resolution_clock::now(); \
{ \
auto d = std::chrono::duration_cast<std::chrono::milliseconds>( __TIMER##NAME##_END - __TIMER##NAME##_START ).count(); \
std::cout << "Timer " << #NAME << ": " << (d / 1000.0) << "s" << std::endl; \
}

