#pragma once

#include <chrono>
#include <iostream>

#ifndef NDEBUG
#	define CSTIMER(message) Timer timer = Timer(message);
#else
#   define CSTIMER(message) // Timer timer = Timer(message);
#endif

struct Timer
{
	std::chrono::time_point<std::chrono::high_resolution_clock> start_point;

	const char* message;

	Timer(const char* _message) : message(_message)
	{
		start_point = std::chrono::high_resolution_clock::now();
	}

	~Timer()
	{
		using namespace std::chrono;
		auto end_point = high_resolution_clock::now();
		auto start = time_point_cast<microseconds>(start_point).time_since_epoch().count();
		auto end = time_point_cast<microseconds>(end_point).time_since_epoch().count();
		auto _duration = end - start;
		std::cout << message << (_duration * 0.001) << "ms" << std::endl;
	}
};