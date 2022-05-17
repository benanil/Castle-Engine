#pragma once

namespace Time
{
	float GetDeltaTime();
	float GetTimeSinceStartup();

	void Tick(unsigned int ticks);
}

