#pragma once
#include "SDL.h"
#include "glm/glm.hpp"

enum class KeyCode
{
    A = 'a', B = 'b', C = 'c', D = 'd', E = 'e', F = 'f', G = 'g', H = 'h', I = 'i', J = 'j', K = 'k', L = 'l', M = 'm', N = 'n', O = 'o', P = 'p',
    Q = 'q', R = 'r', S = 's', T = 't', U = 'u', V = 'v', W = 'w', X = 'x', Y = 'y', Z = 'z',
};

enum class MouseButton 
{
	Right  = SDL_BUTTON_RIGHT,
	Middle = SDL_BUTTON_MIDDLE,
	Left   = SDL_BUTTON_LEFT
};

namespace Input
{
	void Proceed(SDL_Event* event, bool& done);

	// ---INPUT---
    bool GetKeyDown(KeyCode keycode);
    bool GetKeyUp  (KeyCode keycode);
	bool GetMouseButtonDown(MouseButton buttonName);
	bool GetMouseButtonUp(MouseButton buttonName);

	/// <summary> mouse pos relative to window </summary>
	glm::ivec2 GetWindowMousePos();
	/// <summary> mouse pos relative to monitor </summary>
	glm::ivec2 GetMonitorMousePos();

	void SetCursor(SDL_Cursor* _cursor); 
	SDL_Cursor* GetCursor() ;
}
