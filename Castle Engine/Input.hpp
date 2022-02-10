#pragma once
#include "SDL.h"

enum class KeyCode
{
    A = 'a', B = 'b', C = 'c', D = 'd', E = 'e', F = 'f', G = 'g', H = 'h', I = 'i', J = 'j', K = 'k', L = 'l', M = 'm', N = 'n', O = 'o', P = 'p',
    Q = 'q', R = 'r', S = 's', T = 't', U = 'u', V = 'v', W = 'w', X = 'x', Y = 'y', Z = 'z',
};

namespace Input
{
	void Proceed(SDL_Event* event, bool& done);

	// ---INPUT---
	bool GetKeyDown(int keycode);
	bool GetKeyUp(int keycode)	;

    bool GetKeyDown(KeyCode keycode);
    bool GetKeyUp  (KeyCode keycode);

	bool GetMouseButtonDown(int buttonName);
	bool GetMouseButtonUp(int buttonName)  ;

	void SetCursor(SDL_Cursor* _cursor); 
	SDL_Cursor* GetCursor() ;
}
