#pragma once
#include "Helper.hpp"

#ifndef NEDITOR
	#include "ImGui.h"
#endif

class FreeCamera
{
public:
	float aspectRatio; 
	float fov;
	float nearPlane; 
	float farPlane;
	XMMATRIX ViewProjection;
public:
	FreeCamera(float _fov, float _aspectRatio, float _near, float _far)
	:	fov(XM_DegToRad(_fov)), aspectRatio(_aspectRatio), nearPlane(_near), farPlane(_far),
		Up(glm::vec3(0,1,0)), Position(glm::vec3(500, 140, 0))	
	{
		UpdateProjection(_aspectRatio);
		Update(0.0f);
	}

	~FreeCamera();

	void Update(const float& dt)
	{
		auto camPosition = XMVectorSet(GLM_GET_XYZ(Position), 0.0f);
		auto camTarget = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
		auto camUp   = XMVectorSet(GLM_GET_XYZ(Up), 0.0f);

		View = XMMatrixLookAtLH(camPosition, camTarget, camUp);
		ViewProjection = View * Projection;
	}

	void UpdateProjection(const float& _aspectRatio)
	{
		Projection = XMMatrixPerspectiveFovLH(fov, _aspectRatio, nearPlane, farPlane);
	}

	void EditorUpdate()
	{
#ifndef NEDITOR
		ImGui::DragFloat3("Cam Position", glm::value_ptr(Position), 0.01f);
#endif	
	}

private:

	float pitch, yaw;

	glm::vec3 Position;
	glm::vec3 Up;
	glm::vec3 Target;

	XMMATRIX View;
	XMMATRIX Projection;
	
};
