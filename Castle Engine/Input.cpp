#include "Input.hpp"
#include <map>
#define LAMBDAR(x) { return x;  }

namespace Input
{
	std::map<int, bool> keyboard;
	std::map<int, bool> mouse;
	SDL_Cursor* cursor;
}

void Input::Proceed(SDL_Event* event, bool& done)
{
    switch (event->type)
    {
    case SDL_KEYDOWN: keyboard[event->key.keysym.sym] = true; break;
    case SDL_KEYUP:   keyboard[event->key.keysym.sym] = false; break;
    }

    switch (event->button.type)
    {
    case SDL_MOUSEBUTTONDOWN: mouse[event->button.button] = true; break;
    case SDL_MOUSEBUTTONUP:   mouse[event->button.button] = false; break;
    }
	done |= event->type == SDL_QUIT;
}

bool Input::GetKeyDown(KeyCode keycode) { return keyboard[(int)keycode] ; }
bool Input::GetKeyUp(KeyCode keycode)   { return !keyboard[(int)keycode]; }
                                                                     
bool Input::GetKeyDown(int keycode) LAMBDAR(keyboard[keycode] )
bool Input::GetKeyUp(int keycode)   LAMBDAR(!keyboard[keycode])

bool Input::GetMouseButtonDown(int buttonName) LAMBDAR(mouse[buttonName])
bool Input::GetMouseButtonUp(int buttonName) LAMBDAR(mouse[buttonName])

void Input::SetCursor(SDL_Cursor* _cursor) { cursor = _cursor; }
SDL_Cursor* Input::GetCursor() LAMBDAR(cursor);
