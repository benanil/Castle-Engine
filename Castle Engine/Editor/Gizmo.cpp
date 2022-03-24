#include "Gizmo.hpp"
#include <iostream>
#include <utility>
#include "../Input.hpp"
#include "../Rendering/LineDrawer.hpp"
#include "../Rendering/Line2D.hpp"
#include "spdlog/spdlog.h"

using namespace Gizmo;
using namespace CMath;

namespace Gizmo
{
	constexpr int NoIntersection = -1;
	constexpr float ScaleByDistance = 0.23f;

	enum struct Axis  { Forward, Up, Right, Red = 0, Green = 1, Blue = 2 };
	
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
	void ManipulateScale   (const glm::vec3& position, const XMVECTOR& rotation, const XMVECTOR& scale, const XMVECTOR& positionVec, XMMATRIX* matrix);
	void ManipulateRotation(const glm::vec3& position, const XMVECTOR& rotation, const XMVECTOR& scale, const XMVECTOR& posvec, XMMATRIX* matrix);
	
	ArrowResult ArrowStart(const glm::vec3& position, float distToCamera);
	void ArrowEnd(const glm::vec3& position, float distToCamera, int intersection);

	bool OneDimensionalIntersect(float x, const glm::vec2& range);
}

void Gizmo::SetMode(Mode _mode) { mode = _mode; } // depricated
void Gizmo::Initialize(FreeCamera* _freeCamera) { freeCamera = _freeCamera; }

void Gizmo::Begin(
	const glm::vec2& windowScale, const glm::vec2& _mousePosition,
	const XMMATRIX& projection, const XMMATRIX& viewMatrix)
{
	mouseOldPosition = mousePosition;
	mousePosition = _mousePosition;
}

void Gizmo::Manipulate(XMMATRIX* matrix, glm::vec3* position, xmQuaternion* quaternion, glm::vec3* scale)
{
	XMVECTOR positionvec, scaleVec;
	XMVECTOR& _quaternion = *reinterpret_cast<XMVECTOR*>(quaternion);

	XMMatrixDecompose(&scaleVec, &_quaternion, &positionvec, *matrix);
	GetVec3(position, positionvec);

	if (Input::GetKeyDown(KeyCode::W)) { mode = Mode::Position; }
	if (Input::GetKeyDown(KeyCode::Q)) { mode = Mode::Rotation; }
	if (Input::GetKeyDown(KeyCode::E)) { mode = Mode::Scale; }

	switch (mode)
	{
	case Mode::Position: ManipulatePosition(*position, *matrix); break;
	case Mode::Rotation: ManipulateRotation(*position, _quaternion, scaleVec, positionvec, matrix); break;
	case Mode::Scale:    ManipulateScale   (*position, _quaternion, scaleVec, positionvec, matrix); break;
	}

	XMMatrixDecompose(&scaleVec, &_quaternion, &positionvec, *matrix);
	GetVec3(position, positionvec);
	GetVec3(scale, scaleVec);
}

#define CS_GOTO(_name) goto _name;{ // I wrote this because visual studio adds extra tab
#define CS_GOTO_END(_name)   } _name:

void Gizmo::ManipulatePosition(const glm::vec3& position, XMMATRIX& matrix)
{
	float distToCamera = glm::distance(position, freeCamera->GetTransform().GetPosition());
	
	if (Input::GetMouseButtonDown(MouseButton::Left))
	{
		constexpr float speed = 0.02f / 8.6f; // hacky solution for increasing speed over camera distance
		ArrowResult arrowRes = ArrowStart(position, distToCamera);

		if (arrowRes.intersection == NoIntersection) CS_GOTO(finded)
		bool lookingForward = freeCamera->GetTransform().GetForward().x > 0;
		float multiplier = lookingForward || arrowRes.lookingUp || arrowRes.axis == Axis::Red ? -1.0f : 1.0f;
		float direction = arrowRes.lookingUp || arrowRes.axis == Axis::Up ? mouseOldPosition.y - mousePosition.y : mouseOldPosition.x - mousePosition.x;
		if (arrowRes.axis == Axis::Up && lookingForward) direction = -direction;

		matrix.r[3].m128_f32[arrowRes.index] += direction * multiplier * (speed * distToCamera);
		CS_GOTO_END(finded)

		ArrowEnd(position, distToCamera, arrowRes.intersection);
		return;
	}
	ArrowEnd(position, distToCamera, NoIntersection);
}

void Gizmo::ManipulateScale(const glm::vec3& position, const XMVECTOR& rotation, const XMVECTOR& scale, const XMVECTOR& posvec, XMMATRIX* matrix)
{
	float distToCamera = glm::distance(position, freeCamera->GetTransform().GetPosition());
	if (Input::GetMouseButtonDown(MouseButton::Left))
	{
		ArrowResult arrowRes = ArrowStart(position, distToCamera);
		if (arrowRes.intersection != NoIntersection) 
		{
			glm::vec4 axis{};
			float direction = arrowRes.axis == Axis::Up ? mouseOldPosition.y - mousePosition.y : mousePosition.x - mouseOldPosition.x;
			axis[(int)arrowRes.axis] = direction * 0.022;
			XMVECTOR scaleAdition = XMVectorAdd(scale, _mm_load_ps(&axis.x));
			*matrix = XMMatrixScalingFromVector(scaleAdition) * XMMatrixRotationQuaternion(rotation) * XMMatrixTranslationFromVector(posvec);
		}
		ArrowEnd(position, distToCamera, arrowRes.intersection);
		return;
	}
	ArrowEnd(position, distToCamera, NoIntersection);
}

// comining
void Gizmo::ManipulateRotation(const glm::vec3& position, const XMVECTOR& rotation, const XMVECTOR& scale, const XMVECTOR& posvec, XMMATRIX* matrix)
{
	static bool WasIntersecting = false;
	static Bool2 currentDirection = Bool2();

	glm::vec2 ndcPos = freeCamera->WorldToNDC(position);
	glm::vec2 ndcMouse = Input::GetNDC_MousePos();
	ndcMouse.y += 0.03f;

	bool mouseDown = Input::GetMouseButtonDown(MouseButton::Left);
	bool hovering = glm::distance(ndcMouse, ndcPos) < 0.2f;
	bool intersection = hovering && mouseDown;

	if (!WasIntersecting && intersection) {
		glm::vec2 mouseDir = ndcPos - ndcMouse;
		bool isX = fabs(mouseDir.x) > fabs(mouseDir.y);
		currentDirection.x = isX; 
		currentDirection.y = !isX; 
		std::cout << "isx: " << isX ? "true\n" : "false\n";
	}

	WasIntersecting = intersection || (WasIntersecting && mouseDown);
	intersection |= WasIntersecting;

	float distToCamera = ScaleByDistance* glm::distance(position, freeCamera->GetTransform().GetPosition());

	const Color32& oldColor = LineDrawer2D::GetColor();
	LineDrawer::SetColor(intersection ? Color32::Orange() : Color32::Red());
	LineDrawer::DrawCircale(position, distToCamera);
	LineDrawer::SetColor(oldColor);
	
	if (intersection)
	{
		glm::vec2 mouseSubtraction = mousePosition - mouseOldPosition;
		VecMulBool(&mouseSubtraction, currentDirection);
		XMVECTOR axis = XMVectorSet(0, mouseSubtraction.x * 0.2f, mouseSubtraction.y * 0.2f, 0);
		
		if (!XMVector3Equal(axis, XMVectorZero()) && !XMVector3IsInfinite(axis))
		{
			XMVECTOR rotateAdition = XMQuaternionMultiply(rotation, XMQuaternionRotationAxis(axis, mouseSubtraction.length() * 0.066f));
			*matrix = XMMatrixScalingFromVector(scale) * XMMatrixRotationQuaternion(rotateAdition) * XMMatrixTranslationFromVector(posvec);
		}
	}
}

bool Gizmo::OneDimensionalIntersect(float x, const glm::vec2& range)
{
	return x > Min(range.x, range.y) && x < Max(range.x, range.y);
}

ArrowResult Gizmo::ArrowStart(const glm::vec3& position, float distToCamera)
{
	ArrowResult result;
	result.intersection = NoIntersection;
	
 	glm::vec2 ndcPos = freeCamera->WorldToNDC(position);
	glm::vec2 ndcMouse = Input::GetNDC_MousePos();
	ndcMouse.y += 0.034f;

	LineDrawer2D::DrawLine(ndcMouse - glm::vec2(0.02f, 0.00f), ndcMouse + glm::vec2(0.02f, 0.00f));
	LineDrawer2D::DrawLine(ndcMouse - glm::vec2(0.00f, 0.02f), ndcMouse + glm::vec2(0.00f, 0.02f));

	float scale = ScaleByDistance * distToCamera;

	Line2D lineX = freeCamera->Line3DTo2D(Line(position, position + glm::vec3(scale, 0, 0)));
	Line2D lineY = freeCamera->Line3DTo2D(Line(position, position + glm::vec3(0, scale, 0)));
	Line2D lineZ = freeCamera->Line3DTo2D(Line(position, position + glm::vec3(0, 0, scale)));

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

void Gizmo::ArrowEnd(const glm::vec3& position, float distToCamera, int intersection)
{
	const Color32& oldColor = LineDrawer2D::GetColor();
	float scale = ScaleByDistance * distToCamera;

	LineDrawer2D::SetColor(intersection >= 0 != NoIntersection && intersection == (int)Axis::Red ? Color32::Orange() : Color32::Red());
	LineDrawer2D::DrawThickArrow(position, position + glm::vec3(scale , 0, 0), scale * 0.8f);

	LineDrawer2D::SetColor(intersection >= 0 && intersection == (int)Axis::Green ? Color32::Orange() : Color32::Green());
	LineDrawer2D::DrawThickArrow(position, position + glm::vec3(0, scale , 0), scale * 0.8f);
	
	LineDrawer2D::SetColor(intersection >= 0 && intersection == (int)Axis::Blue ? Color32::Orange() : Color32::Blue());
	LineDrawer2D::DrawThickArrow(position, position + glm::vec3(0, 0, scale), scale * 0.8f);
	
	LineDrawer2D::SetColor(oldColor);
}

