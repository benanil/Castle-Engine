#include "Time.hpp"

namespace Time
{
	float DeltaTime;
	float TimeSinceStartup;
	unsigned int lastTime;
}

float Time::GetDeltaTime() { return DeltaTime; }
float Time::GetTimeSinceStartup() { return lastTime; }


void Time::Tick(unsigned int ticks)
{
	DeltaTime = (lastTime - (float)ticks) / 1000.0f;
	lastTime = ticks;
	TimeSinceStartup += DeltaTime;
}