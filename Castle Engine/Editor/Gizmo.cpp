#include "Gizmo.hpp"
#include <iostream>
#include <utility>
#include "../Input.hpp"
#include "../Rendering/LineDrawer.hpp"
#include "../Rendering/Line2D.hpp"
#include "spdlog/spdlog.h"

namespace Gizmo
{
	constexpr int NoIntersection = -1;

	enum class Axis { Forward, Up, Right, Red = 0, Green = 1, Blue = 2 };
	
	struct ArrowResult {
		union { // using one integer as: index, intersection, axis, axis color
			int index; 
			struct { int intersection; };
			struct { Axis axis; };
		};
		bool lookingUp;
	};

	typedef bool SphereResult;

	glm::vec2 mouseOldPosition;
	glm::vec2 mousePosition;
	Mode mode = Mode::Position;
	FreeCamera* freeCamera;

	void ManipulatePosition(const glm::vec3& position, XMMATRIX& matrix);
	void ManipulateScale   (const glm::vec3& position, const XMVECTOR& rotation, const XMVECTOR& scale, const XMVECTOR& positionVec, XMMATRIX& matrix);
	void ManipulateRotation(const glm::vec3& position, const XMVECTOR& rotation, const XMVECTOR& scale, const XMVECTOR& posvec, XMMATRIX& matrix);
	
	ArrowResult ArrowStart(const glm::vec3& position, XMMATRIX& matrix);
	void ArrowEnd(const glm::vec3& position, int intersection);

	SphereResult SphereStart(const glm::vec3& position, XMMATRIX& matrix);
	void SphereEnd(const glm::vec3& position, bool intersection);
	
	bool OneDimensionalIntersect(float x, const glm::vec2& range);
	bool CircaleIntersect(const glm::vec2& pos, const glm::vec2& pos1, float circumferance);
}

void Gizmo::SetMode(Mode _mode) { mode = _mode; } // depricated

using namespace Gizmo;

void Gizmo::Initialize(FreeCamera* _freeCamera) { freeCamera = _freeCamera; }

void Gizmo::Begin(
	const glm::vec2& windowScale, const glm::vec2& _mousePosition,
	const XMMATRIX& projection, const XMMATRIX& viewMatrix)
{
	mouseOldPosition = mousePosition;
	mousePosition = _mousePosition;
}

void Gizmo::Manipulate(XMMATRIX& matrix)
{
	XMVECTOR rotationVec, positionvec, scaleVec;
	glm::vec3 position; 

	XMMatrixDecompose(&scaleVec, &rotationVec, &positionvec, matrix);
	GetVec3(&position, positionvec);

	if (Input::GetKeyDown(KeyCode::W)) { mode = Mode::Position; }
	if (Input::GetKeyDown(KeyCode::Q)) { mode = Mode::Rotation; }
	if (Input::GetKeyDown(KeyCode::E)) { mode = Mode::Scale; }

	switch (mode)
	{
	case Mode::Position: ManipulatePosition(position, matrix); break;
	case Mode::Rotation: ManipulateRotation(position, rotationVec, scaleVec, positionvec, matrix); break;
	case Mode::Scale:    ManipulateScale   (position, rotationVec, scaleVec, positionvec, matrix); break;
	}
}

void Gizmo::ManipulatePosition(const glm::vec3& position, XMMATRIX& matrix)
{
	if (Input::GetMouseButtonDown(MouseButton::Left))
	{
		ArrowResult arrowRes = ArrowStart(position, matrix);

		bool lookingForward = freeCamera->GetTransform().GetForward().x > 0;
		float multiplier = lookingForward || arrowRes.lookingUp || arrowRes.axis == Axis::Red ? -1.0f : 1.0f;
		float direction = arrowRes.lookingUp || arrowRes.axis == Axis::Up ? mouseOldPosition.y - mousePosition.y : mouseOldPosition.x - mousePosition.x;
		if (arrowRes.axis == Axis::Up && lookingForward) direction = -direction;

		matrix.r[3].m128_f32[arrowRes.index] += direction * multiplier * 0.02f;

		ArrowEnd(position, arrowRes.intersection);
		return;
	}
	ArrowEnd(position, NoIntersection);
}

void Gizmo::ManipulateScale(const glm::vec3& position, const XMVECTOR& rotation, const XMVECTOR& scale, const XMVECTOR& posvec, XMMATRIX& matrix)
{
	if (Input::GetMouseButtonDown(MouseButton::Left))
	{
		ArrowResult arrowRes = ArrowStart(position, matrix);
		if (arrowRes.intersection >= 0) 
		{
			glm::vec4 axis{};
			float direction = arrowRes.axis == Axis::Up ? mouseOldPosition.y - mousePosition.y : mousePosition.x - mouseOldPosition.x;
			axis[(int)arrowRes.axis] = direction * 0.02f;
			XMVECTOR scaleAdition = XMVectorAdd(scale, _mm_load_ps(&axis.x));
			matrix = XMMatrixScalingFromVector(scaleAdition) * XMMatrixRotationQuaternion(rotation) * XMMatrixTranslationFromVector(posvec);
		}
		ArrowEnd(position, arrowRes.intersection);
		return;
	}
	ArrowEnd(position, NoIntersection);
}

// comining
void Gizmo::ManipulateRotation(const glm::vec3& position, const XMVECTOR& rotation, const XMVECTOR& scale, const XMVECTOR& posvec, XMMATRIX& matrix)
{
	if (Input::GetMouseButtonDown(MouseButton::Left))
	{
		SphereResult sphereIntersection = SphereStart(position, matrix);
		if (sphereIntersection)
		{
			XMVECTOR axis = XMVectorSet(mousePosition.x - mouseOldPosition.x + 0.001f, mousePosition.y - mouseOldPosition.y + 0.001f, 0, 0);
			XMVECTOR rotateAdition = rotation * XMQuaternionRotationAxis(axis, 0.5f);
			matrix = XMMatrixScalingFromVector(scale) * XMMatrixRotationQuaternion(rotateAdition) * XMMatrixTranslationFromVector(posvec);
		}
		SphereEnd(position, sphereIntersection);
		return;
	}
	SphereEnd(position, NoIntersection);
}

bool Gizmo::OneDimensionalIntersect(float x, const glm::vec2& range)
{
	return x > Min(range.x, range.y) && x < Max(range.x, range.y);
}

bool Gizmo::CircaleIntersect(const glm::vec2& pos, const glm::vec2& pos1, float circumferance)
{
	return glm::distance(pos, pos1) - (circumferance * 2) < 0.0f;
}

SphereResult Gizmo::SphereStart(const glm::vec3& position, XMMATRIX& matrix)
{
	glm::vec2 ndcPos = freeCamera->WorldToNDC(position);
	glm::vec2 ndcMouse = Input::GetNDC_MousePos();
	ndcMouse.y += 0.03f;
	return CircaleIntersect(ndcMouse, ndcPos, 0.1f);
}

void Gizmo::SphereEnd(const glm::vec3& position, bool intersection)
{
	const Color32& oldColor = LineDrawer2D::GetColor();
	LineDrawer::SetColor(intersection ? Color32::Orange() : Color32::Red());
	LineDrawer::DrawCircale(position, 2);
}

ArrowResult Gizmo::ArrowStart(const glm::vec3& position, XMMATRIX& matrix)
{
	ArrowResult result;
	result.intersection = NoIntersection;
	
 	glm::vec2 ndcPos = freeCamera->WorldToNDC(position);
	glm::vec2 ndcMouse = Input::GetNDC_MousePos();
	ndcMouse.y += 0.034f;

	LineDrawer2D::DrawLine(ndcMouse - glm::vec2(0.02f, 0.00f), ndcMouse + glm::vec2(0.02f, 0.00f));
	LineDrawer2D::DrawLine(ndcMouse - glm::vec2(0.00f, 0.02f), ndcMouse + glm::vec2(0.00f, 0.02f));

	Line2D lineX = freeCamera->Line3DTo2D(Line(position, position + glm::vec3(1.5f, 0, 0)));
	Line2D lineY = freeCamera->Line3DTo2D(Line(position, position + glm::vec3(0, 1.5f, 0)));
	Line2D lineZ = freeCamera->Line3DTo2D(Line(position, position + glm::vec3(0, 0, 1.5f)));

	glm::vec3 camForward = freeCamera->transform.GetForward();

	struct Intersect { char intersects; float dist; /* min dist to mouse pos */ };
	Intersect intersects[3];

	result.lookingUp = fabs(camForward.y) > 0.58f;

	if (result.lookingUp ) { // camera looking up or down. we are testing intersection x and z horizontaly
		intersects[0] = { OneDimensionalIntersect(ndcMouse.y, lineX.GetYY()), fabs(ndcMouse.x - lineX.MinX()) };
		intersects[1] = { OneDimensionalIntersect(ndcMouse.x, lineY.GetXX()), fabs(ndcMouse.y - lineY.MinY()) };
		intersects[2] = { OneDimensionalIntersect(ndcMouse.y, lineZ.GetYY()), fabs(ndcMouse.x - lineZ.MinX()) };		
	}
	else { // camera looking straight. we are casting x and z verticaly
		intersects[0] = { OneDimensionalIntersect(ndcMouse.x, lineX.GetXX()), fabs(ndcMouse.y - lineX.MaxY()) };
		intersects[1] = { OneDimensionalIntersect(ndcMouse.y, lineY.GetYY()), fabs(ndcMouse.x - lineY.MaxX()) };
		intersects[2] = { OneDimensionalIntersect(ndcMouse.x, lineZ.GetXX()), fabs(ndcMouse.y - lineZ.MaxY()) };
	}

	constexpr float minIntersectDist = 0.08f;
	float smallestDist = 16.0f;

	for (char i = 0; i < 3; ++i) {

		if (intersects[i].intersects && intersects[i].dist < minIntersectDist && intersects[i].dist < smallestDist) {
			result.index = i; smallestDist = intersects[i].dist;
		}
	}
	return result;
}

void Gizmo::ArrowEnd(const glm::vec3& position, int intersection)
{
	const Color32& oldColor = LineDrawer2D::GetColor();

	LineDrawer2D::SetColor(intersection >= 0 != NoIntersection && intersection == (int)Axis::Red ? Color32::Orange() : Color32::Red());
	LineDrawer2D::DrawThickLine(position, position + glm::vec3(1.5f, 0, 0));
	LineDrawer2D::SetColor(intersection >= 0 && intersection == (int)Axis::Green ? Color32::Orange() : Color32::Green());
	LineDrawer2D::DrawThickLine(position, position + glm::vec3(0, 1.5f, 0));
	LineDrawer2D::SetColor(intersection >= 0 && intersection == (int)Axis::Blue ? Color32::Orange() : Color32::Blue());
	LineDrawer2D::DrawThickLine(position, position + glm::vec3(0, 0, 1.5f));
	LineDrawer2D::SetColor(oldColor);
}

