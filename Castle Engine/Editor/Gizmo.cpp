#include "Gizmo.hpp"
#include "../Input.hpp"
#include "../Rendering/LineDrawer.hpp"
#include <iostream>

namespace Gizmo
{
	constexpr float minInteractionDist = 0.2f; // 0-1 intersection below 0 no intersection so this value must be in range of 0-1
	
	struct ArrowResult {
		int maxIndex; float maxIntersection;
	};

	glm::vec2 mouseOldPosition;
	glm::vec2 mousePosition;
	Line camLine; // actualy ray
	Mode mode = Mode::Position;
	FreeCamera* freeCamera;

	void ManipulatePosition(const glm::vec3& position, XMMATRIX& matrix);
	void ManipulateScale   (const glm::vec3& position, XMMATRIX& matrix);
	void ManipulateRotation(const glm::vec3& position, XMMATRIX& matrix);
	
	ArrowResult ArrowStart(const glm::vec3& position, XMMATRIX& matrix);
	void ArrowEnd(const glm::vec3& position, bool intersection, int maximumIndex);
}

void Gizmo::SetMode(Mode _mode) { mode = _mode; };

using namespace Gizmo;

void Gizmo::Initialize(FreeCamera* _freeCamera) { freeCamera = _freeCamera; }

void Gizmo::Begin(
	const glm::vec2& windowScale, const glm::vec2& _mousePosition,
	const XMMATRIX& projection, const XMMATRIX& viewMatrix)
{
	camLine = LineFromRay(ScreenPointToRay(mousePosition, windowScale, projection, viewMatrix), 25.0f) ;
	mouseOldPosition = mousePosition;
	mousePosition = _mousePosition;
}

void Gizmo::Manipulate(XMMATRIX& matrix)
{
	XMVECTOR temp, positionvec;
	glm::vec3 position; 

	XMMatrixDecompose(&temp, &temp, &positionvec, matrix);
	GetVec3(&position, positionvec);
	switch (mode)
	{
	case Mode::Position: ManipulatePosition(position, matrix); break;
	case Mode::Rotation: ManipulateRotation(position, matrix); break;
	case Mode::Scale:    ManipulateScale   (position, matrix); break;
	}
}

void Gizmo::ManipulatePosition(const glm::vec3& position, XMMATRIX& matrix)
{
	if (Input::GetMouseButtonDown(MouseButton::Left))
	{
		ArrowResult arrowRes = ArrowStart(position, matrix);
		bool intersects = arrowRes.maxIntersection > minInteractionDist;
		if (intersects)
		{
			if      (arrowRes.maxIndex ==  0) { matrix._41 -= (mouseOldPosition.x - mousePosition.x) * 0.01f; }
			else if (arrowRes.maxIndex ==  1) { matrix._42 += (mouseOldPosition.y - mousePosition.y) * 0.01f; }
			else if (arrowRes.maxIndex != -1) { matrix._43 -= (mouseOldPosition.x - mousePosition.x) * 0.01f; }
		}
		ArrowEnd(position, intersects, arrowRes.maxIndex);
		return;
	}
	ArrowEnd(position, -1, -1);
}

void Gizmo::ManipulateScale(const const glm::vec3& position, XMMATRIX& matrix)
{
	if (Input::GetMouseButtonDown(MouseButton::Left))
	{
		ArrowResult arrowRes = ArrowStart(position, matrix);
		bool intersects = arrowRes.maxIntersection > minInteractionDist;
		if (intersects)
		{
			if      (arrowRes.maxIndex  == 0) { matrix *= XMMatrixScaling((mouseOldPosition.x - mousePosition.x) * 0.01f, 0, 0); }
			else if (arrowRes.maxIndex  == 1) { matrix *= XMMatrixScaling(0, (mouseOldPosition.y - mousePosition.y) * 0.01f, 0); }
			else if (arrowRes.maxIndex != -1) { matrix *= XMMatrixScaling(0, 0, (mouseOldPosition.x - mousePosition.x) * 0.01f); }
		}
		ArrowEnd(position, arrowRes.maxIntersection > minInteractionDist, arrowRes.maxIndex);
		return;
	}
	ArrowEnd(position, -1, -1);
}

// comining
void Gizmo::ManipulateRotation(const glm::vec3& position, XMMATRIX& matrix) {
}

ArrowResult Gizmo::ArrowStart(const glm::vec3& position, XMMATRIX& matrix)
{
	ArrowResult result;
	result.maxIntersection = -8.0f;
	result.maxIndex = -1;

	float distances[3] = {
		IntersectionDistanceBetweenLines(camLine, CreateLine(position, position + glm::vec3(3, 0, 0))),
		IntersectionDistanceBetweenLines(camLine, CreateLine(position, position + glm::vec3(0, 3, 0))),
		IntersectionDistanceBetweenLines(camLine, CreateLine(position, position + glm::vec3(0, 0, 3)))
	};

	for (int i = 0; i < 3; ++i) {

		if (distances[i] > 0.0f && distances[i] > result.maxIntersection) {
			result.maxIntersection = distances[i];
			result.maxIndex = i;
		}
	}
	return result;
}

void Gizmo::ArrowEnd(const glm::vec3& position, bool intersection, int maximumIndex)
{
	const glm::vec3& oldColor = LineDrawer::GetColor();

	LineDrawer::SetColor(intersection && maximumIndex == 0 ? glm::vec3(0.6f, 0.6f, 0.2f) : glm::vec3(1, 0, 0));
	LineDrawer::DrawLine(position, position + glm::vec3(1, 0, 0));

	LineDrawer::SetColor(intersection && maximumIndex == 1 ? glm::vec3(0.6f, 0.6f, 0.2f) : glm::vec3(0, 1, 0));
	LineDrawer::DrawLine(position, position + glm::vec3(0, 1, 0));

	LineDrawer::SetColor(intersection && maximumIndex == 2 ? glm::vec3(0.6f, 0.6f, 0.2f) : glm::vec3(0, 0, 1));
	LineDrawer::DrawLine(position, position + glm::vec3(0, 0, 1));

	LineDrawer::SetColor(oldColor);
}

