#include "Input.hpp"
#include "Engine.hpp"
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

glm::ivec2 Input::GetWindowMousePos() 
{
    glm::ivec2 result;
    SDL_GetMouseState(&result.x, &result.y);
    return result;
}

glm::ivec2 Input::GetMonitorMousePos()
{
    glm::ivec2 result;
    SDL_GetGlobalMouseState(&result.x, &result.y);
    return result;
}

glm::vec2 Input::GetNDC_MousePos()
{
	glm::vec2 ndcMouse = (glm::vec2)Input::GetMonitorMousePos() / (glm::vec2)Engine::GetWindowScale();
	ndcMouse = glm::vec2(1.0f, 1.0f) - ndcMouse;
	ndcMouse *= glm::vec2(2.0f, 2.0f);
	ndcMouse -= glm::vec2(1.0f, 1.0f);
	return ndcMouse;
}

bool Input::GetKeyDown(KeyCode keycode) { return keyboard[(int)keycode] ; }
bool Input::GetKeyUp(KeyCode keycode)   { return !keyboard[(int)keycode]; }
                                                                     
bool Input::GetMouseButtonDown(MouseButton buttonName) LAMBDAR(mouse[(int)buttonName])
bool Input::GetMouseButtonUp(MouseButton buttonName) LAMBDAR(!mouse[(int)buttonName])

void Input::SetCursor(SDL_Cursor* _cursor) { cursor = _cursor; }
SDL_Cursor* Input::GetCursor() LAMBDAR(cursor);

