#pragma once
#include <SDL.h>
#include "Transform.hpp"
#include "Math.hpp"

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
	void EditorUpdate();
#endif

	FreeCamera(float _fov, float _aspectRatio, float _near, float _far);
	~FreeCamera();
	void Update();

	glm::vec2 WorldToNDC(const glm::vec3& position);
	glm::vec2 WorldToNDC(const glm::vec3& position, const XMMATRIX& modelMatrix);
	glm::vec2 NDC_ToScreenCoord(const glm::vec2& NDC);
	Line2D Line3DTo2D(const Line& line);

	void SetMatrix();
	void InfiniteMouse(const POINT& point, POINT& oldPos);
	void UpdateProjection();
	void UpdateProjection(const float& _aspectRatio);
};


#undef SET_CURSOR_POS
