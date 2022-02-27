#include "Gizmo.hpp"
#include "../Input.hpp"
#include "../Rendering/LineDrawer.hpp"

namespace Gizmo
{
	glm::vec2 mouseOldPosition;
	glm::vec2 mousePosition;
	Ray camRay;
}

void Gizmo::Begin(
	const glm::vec2& windowScale, const glm::vec2& _mousePosition,
	const XMMATRIX& projection, const XMMATRIX& viewMatrix)
{
	camRay = ScreenPointToRay(mousePosition, windowScale, projection, viewMatrix);
	mouseOldPosition = mousePosition;
	mousePosition = _mousePosition;
}

void Gizmo::Manipulate(XMMATRIX& matrix)
{
	XMVECTOR temp, positionvec;
	glm::vec3 position;

	XMMatrixDecompose(&temp, &temp, &positionvec, matrix);
	GetVec3(&position, positionvec);

	bool intersection = false;
	Ray lineRay = CreateRay(position, glm::vec3(0, 1, 0));

	float dist = glm::distance(camRay.origin, lineRay.origin);

	if (Input::GetMouseButtonDown(MouseButton::Left) && closest_distance_between_lines(camRay, lineRay) / dist < 10.0f)
	{
		intersection = true;
		matrix._43 += (mouseOldPosition.y - mousePosition.y) * 20.0f;
	}

	const glm::vec3& oldColor = LineDrawer::GetColor();
	LineDrawer::SetColor(intersection ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0));
	LineDrawer::DrawLine(position, position + glm::vec3(0, 1, 0));
	LineDrawer::SetColor(oldColor);
}