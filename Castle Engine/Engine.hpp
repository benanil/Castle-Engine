#pragma once
#include "Helper.hpp"
#include <SDL.h>
#include "Main/Event.hpp"

#define DX_CHECK(hr, message) Engine::DirectXCheck(hr, message, __LINE__, RemoveSolutionDir(__FILE__));
#define SDX_CHECK(hr) Engine::DirectXCheck(hr, __LINE__, RemoveSolutionDir((__FILE__)));

// converts BUNCH_OF_PATH/CastleEngine/FILE.xxx to CastleEngine/FILE.xxx 
// sp that means this function creates a compile time string for us
inline constexpr const char* RemoveSolutionDir(const char* file)
{
	constexpr size_t len = __builtin_strlen((__FILE__)) - 24; // 24 is: strlen(CastleEngine/Engine.cpp) 
	return file + len;
}

namespace Engine
{
	DXDevice* GetDevice();
	DXDeviceContext* GetDeviceContext();
	SDL_Window* GetWindow();

	void AddEndOfFrameEvent(const Action& act);

	void DirectXCheck(const HRESULT& hr, const int line, const char* file);
	void DirectXCheck(const HRESULT& hr, const char* message, const int line, const char* file);
}