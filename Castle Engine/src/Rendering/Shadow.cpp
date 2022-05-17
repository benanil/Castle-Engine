// https://github.com/benanil/Zargo--Engine/blob/large/src/Engine/Rendering/Main/Shadow.cs

#include "Shadow.hpp"
#include <exception>
#include "../DirectxBackend.hpp"
#include "../Engine.hpp"
#include "Shader.hpp"
#include "LineDrawer.hpp"
#include "Renderer3D.hpp"

#ifndef NEDITOR
#	include "../Editor/Editor.hpp"
#endif

using namespace CMath;

namespace Shadow
{
	// adsfasdf
	/// <summary> for pbr shader meshes </summary>
	Shader* shader;
	ID3D11DeviceContext* DeviceContext;
	XMMATRIX viewProjection;
	ID3D11Buffer* lightMatrixBuffer;
	// settings
	constexpr int ShadowMapSize = 2048 << 2;
	int OrthoSize = 64;
	float NearPlane = 0.5f;
	float FarPlane = 300.0f;
	float Bias = 0.001f;
	bool ShadowNeedsUpdate = false;
	glm::vec3 OrthoOffset = glm::vec3(0, -200, 0);

	std::array<OrthographicPlane, 4> FrustumDirections;
	glm::vec4 FrustumMinMax;
	RenderTexture* ShadowMap;
	D3D11_VIEWPORT oldViewport;

	void Invalidate();
	void VisualizeOrthographic();
	void CalculateFrustum(float s, float c); // sin cos

#ifndef NEDITOR
	void OnEditor();
#endif
}

void Shadow::UpdateShadows() { ShadowNeedsUpdate = true; }
void Shadow::BindShadowTexture(UINT slot) { ShadowMap->BindDepthTexture(slot); }
const XMMATRIX& Shadow::GetViewProjection() { return viewProjection; }

const std::array<OrthographicPlane, 4>& Shadow::GetFrustumPlanes() { return FrustumDirections; }
const glm::vec4& Shadow::GetFrustumMinMax() { return FrustumMinMax; }

void Shadow::SetShadowMatrix(const XMMATRIX& model, UINT index)
{
	XMMATRIX mvp = XMMatrixTranspose(model * viewProjection);
	DeviceContext->UpdateSubresource(lightMatrixBuffer, 0, NULL, &mvp, 0, 0);
	DeviceContext->VSSetConstantBuffers(index, 1, &lightMatrixBuffer);
}

void Shadow::Initialize()
{
	using RTCreateFlags = RenderTextureCreateFlags;
	// initialize shadow
	DeviceContext = DirectxBackend::GetDeviceContext();
	DXCreateConstantBuffer<XMMATRIX>(DirectxBackend::GetDevice(), lightMatrixBuffer, nullptr);
	shader = new Shader("Shaders/Shadow.hlsl");

	constexpr RTCreateFlags flags = RTCreateFlags::Depth | RTCreateFlags::Linear | RTCreateFlags::NoColor;
	ShadowMap = new RenderTexture(ShadowMapSize, ShadowMapSize, DirectxBackend::GetMSAASamples(), flags);
	Shadow::Invalidate();
#ifndef NEDITOR
	Editor::AddOnEditor(OnEditor);
#endif
}

void Shadow::Invalidate()
{
	ShadowMap->Invalidate(ShadowMapSize, ShadowMapSize);

	float sunAngle = glm::radians(Renderer3D::GetGlobalCbuffer().sunAngle);
	float s = glm::sin(sunAngle);  float c = glm::cos(sunAngle);

	XMVECTOR camCenter  = XMVectorSet(0, s * 250, c * 250, 0);
	XMVECTOR camForward = XMVectorSet(0.0f, -s, -c, 0.0f);
	XMVECTOR camRight   = XMVector3Normalize(XMVector3Cross(camForward, XMVectorSet(0, 1, 0, 0)));
	XMVECTOR camUp      = XMVector3Normalize(XMVector3Cross(camRight, camForward));

	XMMATRIX view = XMMatrixLookAtLH(camCenter, camForward, camUp);
	XMMATRIX projection = XMMatrixOrthographicOffCenterLH(-OrthoSize, OrthoSize, -OrthoSize, OrthoSize, NearPlane, FarPlane);
	viewProjection = view * projection;
	CalculateFrustum(s, c);
	ShadowNeedsUpdate = false;
}

void Shadow::BeginRenderShadowmap()
{
	if (ShadowNeedsUpdate) Invalidate();
	shader->Bind();
	ShadowMap->BindDsvAndSetNullRenderTarget();
	oldViewport = DirectxBackend::GetViewPort();

	D3D11_VIEWPORT newViewPort;
	newViewPort.TopLeftX = 0;
	newViewPort.TopLeftY = 0;
	newViewPort.Width = ShadowMapSize;
	newViewPort.Height = ShadowMapSize;
	newViewPort.MinDepth = 0.0f;
	newViewPort.MaxDepth = 1.0f;

	DeviceContext->RSSetViewports(1, &newViewPort);

	Shadow::VisualizeOrthographic();
}

void Shadow::EndRenderShadowmap(D3D11_VIEWPORT* viewPort)
{
	DeviceContext->RSSetViewports(1, viewPort);
}

void Shadow::VisualizeOrthographic()
{
	float sunAngle = glm::radians(Renderer3D::GetGlobalCbuffer().sunAngle);
	float s = glm::sin(sunAngle); float c = glm::cos(sunAngle);

	glm::vec3 camCenter  = glm::vec3(0.0f, s * 250, c * 250) ;
	glm::vec3 camForward = glm::vec3(0.0f, -s, -c);
	glm::vec3 camRight   = glm::normalize(glm::cross(camForward, glm::vec3(0.0f, 1.0f, 0.0f)));
	glm::vec3 camUp      = glm::normalize(glm::cross(camRight, camForward));

	glm::vec3 nearRightUp   = camCenter + (camRight * float(OrthoSize)) + (camUp * float(OrthoSize));
	glm::vec3 nearLeftUp    = camCenter - (camRight * float(OrthoSize)) + (camUp * float(OrthoSize));
	glm::vec3 nearRightDown = camCenter + (camRight * float(OrthoSize)) - (camUp * float(OrthoSize));
	glm::vec3 nearLeftDown  = camCenter - (camRight * float(OrthoSize)) - (camUp * float(OrthoSize));

	LineDrawer::DrawLine(nearRightUp  , nearRightDown);
	LineDrawer::DrawLine(nearRightDown, nearLeftDown);
	LineDrawer::DrawLine(nearLeftDown , nearLeftUp);
	LineDrawer::DrawLine(nearLeftUp   , nearRightUp);

	LineDrawer::DrawLine(nearRightUp   + (camForward * NearPlane), nearRightUp   + (camForward * FarPlane));
	LineDrawer::DrawLine(nearLeftUp    + (camForward * NearPlane), nearLeftUp    + (camForward * FarPlane));
	LineDrawer::DrawLine(nearRightDown + (camForward * NearPlane), nearRightDown + (camForward * FarPlane));
	LineDrawer::DrawLine(nearLeftDown  + (camForward * NearPlane), nearLeftDown  + (camForward * FarPlane));
}

void Shadow::Dispose() { shader->Dispose(); }

#ifndef NEDITOR
void Shadow::OnEditor()
{
	ImGui::Begin("Shadow");
	ImGui::Image(ShadowMap->depthSRV, { 128, 128 });
	if (ImGui::Button("Update")) Shadow::UpdateShadows();

	// settings
	if (ImGui::DragInt("OrthoSize", &OrthoSize)) UpdateShadows();
	if (ImGui::DragFloat("NearPlane", &NearPlane)) UpdateShadows();
	if (ImGui::DragFloat("FarPlane", &FarPlane)) UpdateShadows();
	if (ImGui::DragFloat("Bias", &Bias)) UpdateShadows();
	if (ImGui::DragFloat3("OrthoOffset", &OrthoOffset.x)) UpdateShadows();

	ImGui::End();
}
#endif

void Shadow::CalculateFrustum(float s, float c)
{
	glm::vec3 camCenter  = glm::vec3(0.0f, s * 250, c * 250) ;
	glm::vec3 camForward = glm::vec3(0.0f, -s, -c);
	glm::vec3 camRight   = glm::normalize(glm::cross(camForward, glm::vec3(0.0f, 1.0f, 0.0f)));
	glm::vec3 camUp      = glm::normalize(glm::cross(camRight, camForward));

	glm::vec3 nearRightUp   = camCenter + (camRight * float(OrthoSize)) + (camUp * float(OrthoSize));
	glm::vec3 nearLeftUp    = camCenter - (camRight * float(OrthoSize)) + (camUp * float(OrthoSize));
	glm::vec3 nearRightDown = camCenter + (camRight * float(OrthoSize)) - (camUp * float(OrthoSize));
	glm::vec3 nearLeftDown  = camCenter - (camRight * float(OrthoSize)) - (camUp * float(OrthoSize));

	glm::vec3 centerOfBox = glm::mix(nearRightUp, nearLeftDown, 0.5f) + (camForward * FarPlane * 0.5f);

	FrustumDirections[0].position = glm::mix(nearRightUp  , nearRightUp   + (camForward * FarPlane), 0.5f); // second parameter is far plane
	FrustumDirections[1].position = glm::mix(nearLeftUp   , nearLeftUp    + (camForward * FarPlane), 0.5f);
	FrustumDirections[2].position = glm::mix(nearLeftDown , nearRightDown + (camForward * FarPlane), 0.5f);
	FrustumDirections[3].position = glm::mix(nearRightDown, nearLeftDown  + (camForward * FarPlane), 0.5f);

	FrustumDirections[0].direction = glm::normalize(centerOfBox - FrustumDirections[0].position);
	FrustumDirections[1].direction = glm::normalize(centerOfBox - FrustumDirections[1].position);
	FrustumDirections[2].direction = glm::normalize(centerOfBox - FrustumDirections[2].position);
	FrustumDirections[3].direction = glm::normalize(centerOfBox - FrustumDirections[3].position);
}
