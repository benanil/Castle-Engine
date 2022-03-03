#pragma once
#define VC_EXTRALEAN    
#define NOMINMAX
#include <d3d11.h>
#include <SDL.h>
#include "Main/Event.hpp"
#undef SDL_HAS_VULKAN 

#define DX_CHECK(hr, message) hr;

// converts BUNCH_OF_PATH/CastleEngine/FILE.xxx to CastleEngine/FILE.xxx 
// sp that means this function creates a compile time string for us
inline constexpr const char* GetRelativePath(const char* file)
{
	constexpr size_t len = __builtin_strlen((__FILE__)) - 24; // 24 is: strlen(CastleEngine/Engine.cpp) 
	return file + len;
}

// these are here because I didn't want to include whole windows.h
struct HWND__;
typedef HWND__* HWND;

namespace Engine
{
	int Width(); int Height();

	SDL_Window* GetWindow();

	void AddEndOfFrameEvent(Action action);
	void AddWindowScaleEvent(FunctionAction<void, int, int>::Type act);
	glm::ivec2 GetWindowScale();
	glm::ivec2 GetMainMonitorScale();

	HWND GetHWND();
}

