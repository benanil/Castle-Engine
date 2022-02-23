#pragma once
#include "Rendering.hpp"
#include <SDL.h>
#include "Transform.hpp"
#include "Main/Time.hpp"
#include "Input.hpp"
#include "Math.hpp"
#include <string>
#ifndef NEDITOR
	#include "Editor/Editor.hpp"
#endif
#include "spdlog/spdlog.h"

class FreeCamera
{
public:
	ECS::Transform transform;
	float aspectRatio; 
	float fov;
	float nearPlane; 
	float farPlane;

	float pitch, yaw;
	float speed = 3, senstivity = 20;

private:
	XMMATRIX View;
	XMMATRIX Projection;
	XMMATRIX ViewProjection;
	POINT oldPos;
	glm::ivec2 monitorScale;
	SDL_Cursor* cursor;
public:

	const XMMATRIX& GetViewProjection()  const { return ViewProjection; }
	const XMMATRIX& GetProjection()      const { return Projection; }
	const XMMATRIX& GetView()            const { return View; }
	const ECS::Transform& GetTransform() const { return transform; }
#ifndef NEDITOR
	void EditorUpdate()
	{
		ImGui::DragFloat("Speed", &speed);
		ImGui::DragFloat("senstivity", &senstivity);
		ImGui::DragFloat("pitch", &pitch);
		ImGui::DragFloat("yaw", &yaw);
		if(ImGui::DragFloat("fov", &fov, 0.01f, 0.1f, 3.0f)) UpdateProjection();
		ImGui::SameLine();
		ImGui::Text(std::to_string(fov * DX_RAD_TO_DEG).c_str());
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
		Update();
		
		SetMatrix();
	}

	~FreeCamera() {};

	void Update()
	{
		uint32_t mouseState = SDL_GetMouseState(nullptr, nullptr);

		float velocity = speed * Time::GetDeltaTime() * ((SDL_GetModState() & KMOD_LSHIFT) ? 8.0f : 2.0f);

		POINT mousePos;
		GetCursorPos(&mousePos);
		
		if (!Input::GetMouseButtonDown(SDL_BUTTON_RIGHT)
#ifndef NEDITOR
			|| !Editor::GameViewWindow::GetData().Focused)
#else
		)
#endif
		{
			oldPos = mousePos;
			Input::SetCursor(nullptr);
			return;
		}

		Input::SetCursor(cursor);
		
		POINT dir{};
		dir.x = mousePos.x - oldPos.x;
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

		if (Input::GetKeyDown(KeyCode::S)) transform.position += transform.GetForward() * velocity;
		if (Input::GetKeyDown(KeyCode::W)) transform.position -= transform.GetForward() * velocity;
		if (Input::GetKeyDown(KeyCode::A)) transform.position -= transform.GetRight() * velocity; 
		if (Input::GetKeyDown(KeyCode::D)) transform.position += transform.GetRight() * velocity; 
		if (Input::GetKeyDown(KeyCode::E)) transform.position -= transform.GetUP() * velocity;    
		if (Input::GetKeyDown(KeyCode::Q)) transform.position += transform.GetUP() * velocity;    

		transform.UpdateTransform();

		SetMatrix();	
		XMMATRIX comboMatrix = XMMatrixTranspose(XMMatrixIdentity() * ViewProjection);
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

	void UpdateProjection()
	{
		UpdateProjection(aspectRatio);
	}

	void UpdateProjection(const float& _aspectRatio)
	{
		aspectRatio = _aspectRatio;
		Projection = XMMatrixPerspectiveFovLH(fov, _aspectRatio, nearPlane, farPlane);
	}
};


#undef SET_CURSOR_POS
