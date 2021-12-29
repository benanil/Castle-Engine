#pragma once
#include "Helper.hpp"
#include <SDL.h>
#include "Transform.hpp"

#ifndef NEDITOR
	#include "ImGui.h"
#endif

class FreeCamera
{
public:
	ECS::Transform transform;
	float aspectRatio; 
	float fov;
	float nearPlane; 
	float farPlane;
	XMMATRIX ViewProjection;

	float pitch, yaw;
	float speed = 50, senstivity = 20;

private:
	XMMATRIX View;
	XMMATRIX Projection;
	POINT oldPos;
	glm::ivec2 monitorScale;
	SDL_Cursor* cursor;
public:
	
#ifndef NEDITOR
	void EditorUpdate()
	{
		ImGui::DragFloat("Speed", &speed);
		ImGui::DragFloat("senstivity", &senstivity);
		ImGui::DragFloat("pitch", &pitch);
		ImGui::DragFloat("yaw", &yaw);
	}
#endif

	FreeCamera(float _fov, float _aspectRatio, float _near, float _far)
		: fov(XM_DegToRad(_fov)), aspectRatio(_aspectRatio), nearPlane(_near), farPlane(_far)
	{
		transform.SetPosition(glm::vec3(500, 140, 0));

		POINT mousePos;
		GetCursorPos(&mousePos);
		oldPos = mousePos;
		cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);

		SDL_DisplayMode DM;
		SDL_GetCurrentDisplayMode(0, &DM);
		monitorScale.x = DM.w;
		monitorScale.y = DM.h;

		UpdateProjection(_aspectRatio);
		Update(0.0f);
		
		SetMatrix();
	}

	~FreeCamera();

	void Update(const float& dt)
	{
		uint32_t mouseState = SDL_GetMouseState(nullptr, nullptr);

		float velocity = speed * (SDL_GetModState() & KMOD_LSHIFT) ? 4.0f : 1.0f;

		POINT mousePos;
		GetCursorPos(&mousePos);
		
		if (!Engine::GetMouseButtonDown(SDL_BUTTON_RIGHT)
#ifndef NEDITOR
			|| !Editor::GameViewWindow::GetData().Focused)
#else
		)
#endif
		{
			oldPos = mousePos;
			Engine::SetCursor(nullptr);
			return;
		}

		Engine::SetCursor(cursor);
		
		POINT dir{};
		dir.x = oldPos.x - mousePos.x;
		dir.y = oldPos.y - mousePos.y;
		
		if (dir.x + dir.y < 150)
		{
			pitch += dir.y  * senstivity * 0.01f;
			yaw += dir.x  * senstivity * 0.01f;
		}
		
		if (pitch > 89.0f)  pitch = 89.0f;
		if (pitch < -89.0f) pitch = -89.0f;
		
		transform.SetEulerDegree({ pitch, xmRepeat(yaw, 360), 0 }, true);
	    
		oldPos = mousePos;
	    
		InfiniteMouse(mousePos, oldPos);

		if (Engine::GetKeyDown(SDLK_w)) transform.position += transform.GetForward() * velocity;
		if (Engine::GetKeyDown(SDLK_s)) transform.position -= transform.GetForward() * velocity;
		if (Engine::GetKeyDown(SDLK_d)) transform.position += transform.GetRight() * velocity; 
		if (Engine::GetKeyDown(SDLK_a)) transform.position -= transform.GetRight() * velocity; 
		if (Engine::GetKeyDown(SDLK_q)) transform.position -= transform.GetUP() * velocity;    
		if (Engine::GetKeyDown(SDLK_e)) transform.position += transform.GetUP() * velocity;    

		transform.UpdateTransform();

		SetMatrix();
	}

	void SetMatrix()
	{
		glm::vec3 lookPos = transform.position + transform.GetForward();

		auto camPosition = XMVectorSet(GLM_GET_XYZ(transform.position), 0.0f);
		auto camTarget = XMVectorSet(GLM_GET_XYZ(lookPos), 0.0f);
		auto camUp = XMVectorSet(GLM_GET_XYZ(transform.GetUP()), 0.0f);

		View = XMMatrixLookAtLH(camPosition, camTarget, camUp);
		ViewProjection = View * Projection;
	}

	void InfiniteMouse(const POINT& point, POINT& oldPos)
	{
		#define SET_CURSOR_POS(_x, _y) { SetCursorPos(_x, _y); oldPos.x = _x; oldPos.y = _y; }

		if (point.x > monitorScale.x - 2) SET_CURSOR_POS(3, point.y);
		if (point.y > monitorScale.y - 2) SET_CURSOR_POS(point.x, 3);
		
		if (point.x < 2) SET_CURSOR_POS(monitorScale.x - 3, point.y);
		if (point.y < 2) SET_CURSOR_POS(point.x, monitorScale.y - 3);
	}

	void UpdateProjection(const float& _aspectRatio)
	{
		aspectRatio = _aspectRatio;
		Projection = XMMatrixPerspectiveFovLH(fov, _aspectRatio, nearPlane, farPlane);
	}
};


#undef SET_CURSOR_POS
