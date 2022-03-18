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

namespace Shadow
{
	/// <summary> for pbr shader meshes </summary>
	Shader* shader;
	ID3D11DeviceContext* DeviceContext;
	XMMATRIX viewProjection;
	ID3D11Buffer* lightMatrixBuffer;
	// settings
	int ShadowMapSize = 2048 << 1;
	int OrthoSize = 65;
	float NearPlane = 0.5f;
	float FarPlane = 400.0f;
	float Bias = 0.001f;
	bool ShadowNeedsUpdate = false;
	glm::vec3 OrthoOffset = glm::vec3(18, -200, 0);

	RenderTexture* ShadowMap;
	D3D11_VIEWPORT oldViewport;

	void Invalidate();
	void VisualizeOrthographic();

#ifndef NEDITOR
	void OnEditor();
#endif
}

const XMMATRIX& Shadow::GetViewProjection() { return viewProjection; }
void Shadow::UpdateShadows() { ShadowNeedsUpdate = true; }
void Shadow::BindShadowTexture(UINT slot) { ShadowMap->BindDepthTexture(slot); }

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

	RTCreateFlags flags = RTCreateFlags::Depth | RTCreateFlags::Linear | RTCreateFlags::NoColor;
	ShadowMap = new RenderTexture(ShadowMapSize, ShadowMapSize, DirectxBackend::GetMSAASamples(), flags);
	Shadow::Invalidate();
#ifndef NEDITOR
	Editor::AddOnEditor(OnEditor);
#endif
}

static void PrintMatrix(const XMMATRIX& matrix)
{
	for (int y = 0; y < 4; ++y)
	{
		for (int x = 0; x < 4; ++x)
		{
			std::cout << matrix.m[x][y] << " ";
		}
		std::cout << std::endl;
	}
}

void Shadow::Invalidate()
{
	ShadowMap->Invalidate(ShadowMapSize, ShadowMapSize);

	float sunAngle = glm::radians(Renderer3D::GetGlobalCbuffer().sunAngle);
	float s = glm::sin(sunAngle);  float c = glm::cos(sunAngle);

	XMVECTOR camCenter  = XMVectorSet(0, s * 250, c * 250, 0);
	XMVECTOR camForward = XMVectorSet(0.0f, -s, -c, 0.0f);
	XMVECTOR camRight   = XMVector3Cross(camForward, XMVectorSet(0, 1, 0, 0));
	XMVECTOR camUp      = XMVector3Cross(camRight, camForward);

	XMMATRIX view = XMMatrixLookAtLH(camCenter, camForward, camUp);
	XMMATRIX projection = XMMatrixOrthographicOffCenterLH(-OrthoSize, OrthoSize, -OrthoSize, OrthoSize, NearPlane, FarPlane);
	viewProjection = view * projection;
	
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
	glm::vec3 camRight   = glm::cross(camForward, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::vec3 camUp      = glm::cross(camRight, camForward);

	glm::vec3 rightUp   = camCenter + (camRight * float(OrthoSize)) + (camUp * float(OrthoSize));
	glm::vec3 leftUp    = camCenter - (camRight * float(OrthoSize)) + (camUp * float(OrthoSize));
	glm::vec3 rightDown = camCenter + (camRight * float(OrthoSize)) - (camUp * float(OrthoSize));
	glm::vec3 leftDown  = camCenter - (camRight * float(OrthoSize)) - (camUp * float(OrthoSize));

	LineDrawer::DrawLine(rightUp  , rightDown);
	LineDrawer::DrawLine(rightDown, leftDown);
	LineDrawer::DrawLine(leftDown , leftUp);
	LineDrawer::DrawLine(leftUp   , rightUp);
	LineDrawer::DrawLine(rightUp   + (camForward * NearPlane), rightUp   + (camForward * FarPlane));
	LineDrawer::DrawLine(rightDown + (camForward * NearPlane), rightDown + (camForward * FarPlane));
	LineDrawer::DrawLine(leftDown  + (camForward * NearPlane), leftDown  + (camForward * FarPlane));
	LineDrawer::DrawLine(leftUp    + (camForward * NearPlane), leftUp    + (camForward * FarPlane));
}

void Shadow::Dispose() { shader->Dispose(); }

#ifndef NEDITOR
void Shadow::OnEditor()
{
	ImGui::Begin("Shadow");
	ImGui::Image(ShadowMap->depthSRV, { 128, 128 });
	if (ImGui::Button("Update")) Shadow::UpdateShadows();

	// settings
	if (ImGui::DragInt("ShadowMapSize", &ShadowMapSize)) UpdateShadows();
	if (ImGui::DragInt("OrthoSize", &OrthoSize)) UpdateShadows();
	if (ImGui::DragFloat("NearPlane", &NearPlane)) UpdateShadows();
	if (ImGui::DragFloat("FarPlane", &FarPlane)) UpdateShadows();
	if (ImGui::DragFloat("Bias", &Bias)) UpdateShadows();
	if (ImGui::DragFloat3("OrthoOffset", &OrthoOffset.x)) UpdateShadows();

	ImGui::End();
}
#endif