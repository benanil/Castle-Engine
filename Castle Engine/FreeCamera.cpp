#include <string>
#include "FreeCamera.hpp"
#include "Rendering.hpp"
#include "Main/Time.hpp"
#include "Input.hpp"
#include "Math.hpp"
#include "DirectxBackend.hpp"
#ifndef NEDITOR
#	include "Editor/Editor.hpp"
#endif

using namespace CMath;

#ifndef NEDITOR
void FreeCamera::EditorUpdate()
{
	ImGui::DragFloat("Speed", &speed);
	ImGui::DragFloat("senstivity", &senstivity);
	ImGui::DragFloat("pitch", &pitch);
	ImGui::DragFloat("yaw", &yaw);
	if (ImGui::DragFloat("fov", &fov, 0.01f, 0.1f, 3.0f)) UpdateProjection();
	ImGui::SameLine();
	ImGui::Text(std::to_string(fov * DX_RAD_TO_DEG).c_str());
}
#endif

FreeCamera::FreeCamera(float _fov, float _aspectRatio, float _near, float _far)
	: fov(XM_DegToRad(_fov)), aspectRatio(_aspectRatio), nearPlane(_near), farPlane(_far)
{
	transform.SetPosition(glm::vec3(0, 2, 0));

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

FreeCamera::~FreeCamera() {};

void FreeCamera::Update()
{
	float velocity = speed * Time::GetDeltaTime() * ((SDL_GetModState() & KMOD_LSHIFT) ? 8.0f : 2.0f);

	POINT mousePos;
	GetCursorPos(&mousePos);

	if (!Input::GetMouseButtonDown(MouseButton::Right)
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
#ifndef NEDITOR
	dir.x = mousePos.x - oldPos.x;
	dir.y = oldPos.y - mousePos.y;
#else 
	dir.x = oldPos.x - mousePos.x;
	dir.y = oldPos.y - mousePos.y;
#endif 

	if (dir.x + dir.y < 150)
	{
		pitch += dir.y * senstivity * 0.01f;
		yaw += dir.x * senstivity * 0.01f;
	}

	if (pitch > 89.0f)  pitch = 89.0f;
	if (pitch < -89.0f) pitch = -89.0f;

	transform.SetEulerDegree({ pitch, xmRepeat(yaw, 360), 0 }, true);

	oldPos = mousePos;

	InfiniteMouse(mousePos, oldPos);

	if (Input::GetKeyDown(KeyCode::W)) transform.position -= transform.GetForward() * velocity;
	if (Input::GetKeyDown(KeyCode::S)) transform.position += transform.GetForward() * velocity;
	if (Input::GetKeyDown(KeyCode::Q)) transform.position += transform.GetUP() * velocity;
	if (Input::GetKeyDown(KeyCode::E)) transform.position -= transform.GetUP() * velocity;
#ifndef NEDITOR
	if (Input::GetKeyDown(KeyCode::A)) transform.position -= transform.GetRight() * velocity;
	if (Input::GetKeyDown(KeyCode::D)) transform.position += transform.GetRight() * velocity;
#else
	if (Input::GetKeyDown(KeyCode::I)) speed += 1;
	if (Input::GetKeyDown(KeyCode::C)) speed -= 1;

	if (Input::GetKeyDown(KeyCode::A)) transform.position += transform.GetRight() * velocity;
	if (Input::GetKeyDown(KeyCode::D)) transform.position -= transform.GetRight() * velocity;
#endif

	transform.UpdateTransform();

	SetMatrix();
	XMMATRIX comboMatrix = XMMatrixTranspose(XMMatrixIdentity() * ViewProjection);
}

void FreeCamera::SetMatrix()
{
	glm::vec3 lookPos = transform.position + transform.GetForward();

	auto camPosition = XMVectorSet(GLM_GET_XYZ(transform.position), 0.0f);
	auto camTarget = XMVectorSet(GLM_GET_XYZ(lookPos), 0.0f);
	auto camUp = XMVectorSet(GLM_GET_XYZ(transform.GetUP()), 0.0f);

	View = XMMatrixLookAtLH(camPosition, camTarget, camUp);
	ViewProjection = View * Projection;
}

void FreeCamera::InfiniteMouse(const POINT& point, POINT& oldPos)
{
#define SET_CURSOR_POS(_x, _y) { SetCursorPos(_x, _y); oldPos.x = _x; oldPos.y = _y; }

	if (point.x > monitorScale.x - 2) SET_CURSOR_POS(3, point.y);
	if (point.y > monitorScale.y - 2) SET_CURSOR_POS(point.x, 3);

	if (point.x < 2) SET_CURSOR_POS(monitorScale.x - 3, point.y);
	if (point.y < 2) SET_CURSOR_POS(point.x, monitorScale.y - 3);
}

Line2D FreeCamera::Line3DTo2D(const Line& line)
{
	return Line2D(WorldToNDC(line.point1), WorldToNDC(line.point2));
}

glm::vec2 FreeCamera::WorldToNDC(const glm::vec3& position) { return WorldToNDC(position, std::move(XMMatrixIdentity())); }

glm::vec2 FreeCamera::WorldToNDC(const glm::vec3& position, const XMMATRIX& modelMatrix)
{
	glm::vec4 temp {position.x, position.y, position.z, 1};

	XMVECTOR clipCoords = XMVector3Transform(_mm_load_ps(&temp.x), modelMatrix * ViewProjection);

	return glm::vec2 {
		XMVectorGetX(clipCoords) / XMVectorGetW(clipCoords),
		XMVectorGetY(clipCoords) / XMVectorGetW(clipCoords)
	};
}

// convert ndc to screen cords
glm::vec2 FreeCamera::NDC_ToScreenCoord(const glm::vec2& NDC)
{
	D3D11_VIEWPORT ViewPort = DirectxBackend::GetViewPort();
	return glm::vec2(
		(ViewPort.Width / 2 * NDC.x) + (NDC.x + ViewPort.Width / 2),
		-(ViewPort.Height / 2 * NDC.y) + (NDC.y + ViewPort.Height / 2)
	);
}

void FreeCamera::UpdateProjection()
{
	UpdateProjection(aspectRatio);
}

void FreeCamera::UpdateProjection(const float& _aspectRatio)
{
	aspectRatio = _aspectRatio;
	Projection = XMMatrixPerspectiveFovLH(fov, _aspectRatio, nearPlane, farPlane);
}


#undef SET_CURSOR_POS
