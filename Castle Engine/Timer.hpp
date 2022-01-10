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
	std::chrono::time_point<std::chrono::steady_clock> start, end;
	std::chrono::duration<float> duration;
	const char* message;

	Timer(const char* _message) : message(_message)
	{
		start = std::chrono::high_resolution_clock::now();
	}
	~Timer()
	{
		end = std::chrono::high_resolution_clock::now();
		duration = end - start;
		float ms = duration.count() * 1000.0f;
		std::cout << message << ms << "ms" << std::endl;
	}
};