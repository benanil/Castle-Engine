#pragma once
#include <array>
#include <vector>
#include <array>
#include <future>
#include <thread>
#include "../DirectxBackend.hpp"
#include "../Main/Time.hpp"
#include "../Timer.hpp"
#include "Renderer3D.hpp"
#include "Terrain.hpp"
#include "Grassrenderer.hpp"
#include "ComputeShader.hpp"
#include "Shader.hpp"
#include "Skybox.hpp"
#include "TesellatedMesh.hpp"
#include "PostProcessing.hpp"
#include "LineDrawer.hpp"
#include "Line2D.hpp"
#include "GFSDK_SSAO.h"
#include "Shadow.hpp"
#include "../Math.hpp"

#ifndef NEDITOR
#	include "../Editor/Editor.hpp"
#endif

using namespace CMath;

namespace Renderer3D
{
	DXDevice* Device; DXDeviceContext* DeviceContext;

	// post processing
	DXInputLayout* quadVertLayout;
	DXBuffer* ScreenSizeCB;
	ID3D11Buffer* MVP_Cbuffer;

	std::vector<MeshRenderer*> meshRenderers;

	DrawIndexedInfo drawInfo;
	unsigned int MSAASamples;

	DXInputLayout* PBRVertLayout;
	cbGlobal cbGlobalData;
	cbPerObject cbPerObj;

	DXBuffer* constantBuffer, * uniformGlobalBuffer;

	RenderTexture* renderTexture;
	Skybox* skybox;
	Shader* PBRshader;
	ID3D11RasterizerState* rasterizerState, * WireframeRasterizerState;
	TesellatedMesh* tessMesh;
	FreeCamera* freeCamera;

	std::future<CullingBitset> cullingThread;
	FrustumBitset terrainCullBitset;

	bool Vsync = true;
	int culledMeshCount = 0;

	GFSDK_SSAO_Context_D3D11* pAOContext;

	// forward declarations
	void DrawTerrain();
	void CreateBuffers();
	CullingBitset CalculateCulls();
}
// getters
cbGlobal& Renderer3D::GetGlobalCbuffer() { return cbGlobalData; }
RenderTexture* Renderer3D::GetPostRenderTexture() { return PostProcessing::GetPostRenderTexture(); }
FreeCamera* Renderer3D::GetCamera() { return freeCamera; };

// do not send matrix transposed!
void Renderer3D::SetMVP(const XMMATRIX& model, const XMMATRIX& viewProjection)
{
	XMMATRIX mvp = XMMatrixTranspose(model * viewProjection);
	DeviceContext->UpdateSubresource(MVP_Cbuffer, 0, NULL, &mvp, 0, 0);
	DeviceContext->VSSetConstantBuffers(0, 1, &MVP_Cbuffer);
}

// do not send matrix transposed!
void Renderer3D::SetModelMatrix(const XMMATRIX& matrix)
{
	cbPerObj.MVP = XMMatrixTranspose(matrix * freeCamera->GetViewProjection());
	cbPerObj.Model = XMMatrixTranspose(matrix);
	DeviceContext->UpdateSubresource(constantBuffer, 0, NULL, &cbPerObj, 0, 0);
	DeviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);
}

// screen space quad for post processing and other stuff
void Renderer3D::RenderToQuad() {
	DeviceContext->IASetInputLayout(nullptr);
	DeviceContext->IASetIndexBuffer(nullptr, (DXGI_FORMAT)0, 0);
	DeviceContext->IASetInputLayout(nullptr);
	DeviceContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	DeviceContext->Draw(3, 0);
}

void Renderer3D::AddMeshRenderer(MeshRenderer* meshRenderer)
{
	meshRenderers.push_back(meshRenderer);
}

void Renderer3D::Initialize(FreeCamera* camera)
{
	Device = DirectxBackend::GetDevice(); DeviceContext = DirectxBackend::GetDeviceContext();
	freeCamera = camera; MSAASamples = DirectxBackend::GetMSAASamples();

	DXCreateConstantBuffer<XMMATRIX>(DirectxBackend::GetDevice(), MVP_Cbuffer, nullptr);

	GFSDK_SSAO_CustomHeap CustomHeap;
	CustomHeap.new_ = ::operator new;
	CustomHeap.delete_ = ::operator delete;

	if (GFSDK_SSAO_CreateContext_D3D11(Device, &pAOContext, &CustomHeap) != GFSDK_SSAO_OK) {
		throw std::exception("ssao creation failed!");
	}
	PostProcessing::Initialize(Device, DeviceContext, MSAASamples);
	LineDrawer2D::Initialize(freeCamera);
	GrassRenderer::Initialize(Device, DeviceContext);
	LineDrawer::Initialize();
	Shadow::Initialize();
	CreateBuffers();

	PBRshader = new Shader("Shaders/First.hlsl\0");
	PBRshader->Bind();

	Renderer3D::RenderToQuad();

	//Create the Input Layout & Set the Input Layout
	D3D11_INPUT_ELEMENT_DESC pbrLayoutInfos[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 , D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "NORMAL"  , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT   , 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "TANGENT" , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	DirectxBackend::GetDevice()->CreateInputLayout(
		pbrLayoutInfos, 4, PBRshader->VS_Buffer->GetBufferPointer(),
		PBRshader->VS_Buffer->GetBufferSize(), &PBRVertLayout);

	DeviceContext->IASetInputLayout(PBRVertLayout);
	//Set Primitive Topology
	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Create Rasterizer 
	DX_CREATE(D3D11_RASTERIZER_DESC, rasterizerDesc);
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.MultisampleEnable = true;

	DX_CHECK(
		Device->CreateRasterizerState(&rasterizerDesc, &rasterizerState), "rasterizer creation failed");

	memset(&rasterizerDesc, 0, sizeof(D3D11_RASTERIZER_DESC));
	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.MultisampleEnable = true;

	DeviceContext->RSSetState(rasterizerState);

	DX_CHECK(
		Device->CreateRasterizerState(&rasterizerDesc, &WireframeRasterizerState), "rasterizer creation failed");

	skybox = new Skybox(10, 10, MSAASamples);
	std::cout << "skybox created" << std::endl;
	Terrain::Initialize();

	PointsAndIndices32  tessMeshCreateResult = CSCreatePlanePoints(100, 100, { 0,0 });
	tessMesh = new TesellatedMesh(Device, tessMeshCreateResult);
	tessMeshCreateResult.Clear();

	renderTexture = new RenderTexture(Engine::Width(), Engine::Height(), DirectxBackend::GetMSAASamples(), RenderTextureCreateFlags::Depth | RenderTextureCreateFlags::Linear);
	Shadow::UpdateShadows();
}

void Renderer3D::InvalidateRenderTexture(int width, int height)
{
	if (!renderTexture) return;
	renderTexture->Invalidate(width, height);
	freeCamera->aspectRatio = static_cast<float>(width) / height;
	freeCamera->UpdateProjection();
	PostProcessing::WindowScaleEvent((int)width, (int)height);
}

void Renderer3D::CreateBuffers()
{
	cbGlobalData.ambientColor = glm::vec3(0.95f, 0.95f, 0.85f);
	cbGlobalData.sunColor = glm::vec3(1.0f, 1.0f, 1.0f);
	cbGlobalData.ambientStength = .140f;
	cbGlobalData.sunAngle = 110;

	cbPerObj.MVP = XMMatrixTranspose(freeCamera->GetViewProjection());
	cbPerObj.Model = XMMatrixIdentity();

	DXCreateConstantBuffer(Device, uniformGlobalBuffer, &cbGlobalData);
	DXCreateConstantBuffer(Device, constantBuffer, &cbPerObj);

	DeviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);
	DeviceContext->PSSetConstantBuffers(2, 1, &uniformGlobalBuffer);

#ifndef NEDITOR
	Editor::GameViewWindow::GetData().texture = Renderer3D::GetPostRenderTexture()->textureView;

	auto& viewWindowdata = Editor::GameViewWindow::GetData();
	viewWindowdata.OnScaleChanged.Bind(new StaticMethodDel<void, float, float>([](float w, float h)
		{
			freeCamera->aspectRatio = w / h;
			freeCamera->UpdateProjection(freeCamera->aspectRatio);
			renderTexture->Invalidate((int)w, (int)h);
			PostProcessing::WindowScaleEvent((int)w, (int)h);
			Editor::GameViewWindow::GetData().texture = Renderer3D::GetPostRenderTexture()->textureView;
		}));
#endif
}

void Renderer3D::DrawTerrain()
{
	Terrain::BindShader();

	cbGlobalData.additionalData = Terrain::GetTextureScale();
	DeviceContext->UpdateSubresource(uniformGlobalBuffer, 0, NULL, &cbGlobalData, 0, 0);
	DeviceContext->PSSetConstantBuffers(1, 1, &uniformGlobalBuffer);
	terrainCullBitset = Terrain::Draw(*freeCamera, freeCamera->GetViewProjection());

	// 600k grass in 0.07 seconds depends on sea level framerate can increase %20
	GrassRenderer::SetShader(freeCamera->GetViewProjection());
	Terrain::DrawGrasses(terrainCullBitset);
	GrassRenderer::EndRender();

	DeviceContext->RSSetState(rasterizerState);
}


void Renderer3D::RenderMeshes(D3D11_VIEWPORT* viewPort)
{
	if (!cullingThread.valid()) return;

	// get MeshCull data from async thread
	cullingThread.wait();
	CullingBitset PBR_MeshCullBitset = cullingThread.get();

	// DONE todo: orthographic frustum culling for shadowmap. it will improve performance a lot
	// render shadowmap
	Shadow::BeginRenderShadowmap();
	uint32_t cullStartIndex = 1024;

	DeviceContext->IASetInputLayout(PBRVertLayout);

	for (auto& renderer : meshRenderers) {
		renderer->RenderForShadows(DeviceContext, PBR_MeshCullBitset, cullStartIndex);
	}

	Terrain::DrawForShadow(Shadow::GetFrustumMinMax());

	Shadow::EndRenderShadowmap(viewPort); // only sets viewport

	// render pbr scene 
	PBRshader->Bind();
	renderTexture->SetAsRendererTarget();
	Shadow::BindShadowTexture(3);
	DeviceContext->IASetInputLayout(PBRVertLayout);
	DeviceContext->PSSetConstantBuffers(2, 1, &uniformGlobalBuffer);
	cullStartIndex = 0;

	for (auto& renderer : meshRenderers) {
		renderer->Draw(DeviceContext, PBR_MeshCullBitset, cullStartIndex);
	}
	LineDrawer::SetMatrix(XMMatrixIdentity());
}

void Renderer3D::DrawScene()
{
	culledMeshCount = 0;
	cbGlobalData.viewPos = freeCamera->transform.position;
	DeviceContext->UpdateSubresource(constantBuffer, 0, NULL, &cbPerObj, 0, 0);
	DeviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);

	// use additionalData  as time since startup
	cbGlobalData.additionalData = Time::GetTimeSinceStartup();

	DeviceContext->UpdateSubresource(uniformGlobalBuffer, 0, NULL, &cbGlobalData, 0, 0);
	DeviceContext->PSSetConstantBuffers(2, 1, &uniformGlobalBuffer);

	D3D11_VIEWPORT viewPort = DirectxBackend::GetViewPort();
#ifndef NEDITOR
	auto& gameVindowdata = Editor::GameViewWindow::GetData();
	viewPort.Width = gameVindowdata.WindowScale.x;
	viewPort.Height = gameVindowdata.WindowScale.y;

	DeviceContext->RSSetViewports(1, &viewPort);
	renderTexture->SetBlendState();
#endif

	//Clear our backbuffer
	DirectxBackend::ClearBackBuffer();
	float bgColor[4] = { .4, .4, .7, 1.0f };
	// rendering to texture Here
	renderTexture->ClearRenderTarget(bgColor);
	renderTexture->SetAsRendererTarget();

	DeviceContext->RSSetState(rasterizerState);
	Renderer3D::RenderMeshes(&viewPort);

	DeviceContext->RSSetState(WireframeRasterizerState);
	// SetModelMatrix(XMMatrixTranslation(-000, 0, -000) * XMMatrixScaling(7, 7, 7));
	// tessMesh->Render(DeviceContext, cbPerObj.MVP, freeCamera->transform.GetPosition());
	DeviceContext->RSSetState(rasterizerState);

	LineDrawer::SetShader();
	SetModelMatrix(XMMatrixTranslation(-000, 0, -000) * XMMatrixScaling(1, 1, 1));
	LineDrawer::Render();

	LineDrawer2D::Render();

	DrawTerrain();

	skybox->Draw(cbPerObj, DeviceContext, constantBuffer, freeCamera);

#ifndef NEDITOR
	PostProcessing::Proceed(*renderTexture, freeCamera);
#else
	PostProcessing::Proceed(*renderTexture, freeCamera);
#endif
	// set default render buffers again
	DeviceContext->OMSetDepthStencilState(nullptr, 0);
	DeviceContext->RSSetState(rasterizerState);

	DirectxBackend::SetBackBufferAsRenderTarget();
	DeviceContext->IASetInputLayout(PBRVertLayout);

	PBRshader->Bind();
#ifndef NEDITOR
	viewPort = DirectxBackend::GetViewPort();
	DeviceContext->RSSetViewports(1, &viewPort);
	Editor::Render(); // Draw Imgui
#endif

	//Present the backbuffer to the screen
	DirectxBackend::Present(Vsync);

	freeCamera->Update();
	cullingThread = std::async(std::launch::async, CalculateCulls);
}

__forceinline float __vectorcall Dot(__m128 a, __m128 b)
{
	__m128 r1 = _mm_mul_ps(a, b);
	__m128 shuf = _mm_shuffle_ps(r1, r1, _MM_SHUFFLE(2, 3, 0, 1));
	__m128 sums = _mm_add_ps(r1, shuf);
	shuf = _mm_movehl_ps(shuf, sums);
	sums = _mm_add_ss(sums, shuf);
	return _mm_cvtss_f32(sums);
}
__forceinline __m128 __vectorcall Normalize(__m128 V)
{
	__m128 vDot = _mm_mul_ps(V, V);
	__m128 vTemp = _mm_shuffle_ps(vDot, vDot, _MM_SHUFFLE(2, 1, 2, 1));
	vDot = _mm_add_ss(vDot, vTemp);
	vTemp = _mm_shuffle_ps(vTemp, vTemp, _MM_SHUFFLE(1, 1, 1, 1));
	vDot = _mm_add_ss(vDot, vTemp);
	vDot = _mm_shuffle_ps(vDot, vDot, _MM_SHUFFLE(0, 0, 0, 0));
	return _mm_mul_ps(_mm_rsqrt_ps(vDot), V);
}

XMGLOBALCONST XMVECTORI32 g_XMSelect0010 = { XM_SELECT_0, XM_SELECT_0, XM_SELECT_1, XM_SELECT_0 };
XMGLOBALCONST XMVECTORI32 g_XMSelect0100 = { XM_SELECT_0, XM_SELECT_1, XM_SELECT_0, XM_SELECT_0 };
XMGLOBALCONST XMVECTORI32 g_XMSelect0110 = { XM_SELECT_0, XM_SELECT_1, XM_SELECT_1, XM_SELECT_0 };

// this tooks 0.23ms-0.7ms runs asyncrusly on seperate thread
CullingBitset Renderer3D::CalculateCulls() // angle culling (like frustum culling)
{
	CullingBitset bitset{};

	const XMMATRIX viewProjection = freeCamera->GetViewProjection();
	const std::array<OrthographicPlane, 4> shadowFrustum = Shadow::GetFrustumPlanes();
	const CMath::BoundingFrustum frustum = CMath::BoundingFrustum(viewProjection);
	
	struct SSEPlane { __m128 position, direction; } planeSSE[4];
	
	for (int i = 0; i < 4; ++i)
	{
		planeSSE[i].position  = _mm_loadu_ps(&shadowFrustum[i].position.x);
		planeSSE[i].direction = _mm_loadu_ps(&shadowFrustum[i].direction.x);
	}
	
	static const __m128 const edgeIndices[8] =
	{
		g_XMSelect1110, _mm_setzero_ps(), g_XMSelect1100, g_XMSelect0010,
		g_XMSelect1010, g_XMSelect0100, g_XMSelect1000, g_XMSelect0110
	};
	
	int count = 0, bi = 0; // bitset index
	
	for (const MeshRenderer* meshRenderer : meshRenderers)
	{
		for (const SubMesh& submesh : meshRenderer->mesh->subMeshes)
		{
			const CMath::AABB& aabb = submesh.aabb;
	
			const XMMATRIX& enttMatrix = meshRenderer->GetEntityConst()->transform->GetMatrix();
	
			__m128 min = XMVector3Transform(_mm_loadu_ps(&aabb.min.x), enttMatrix);
			__m128 max = XMVector3Transform(_mm_loadu_ps(&aabb.max.x), enttMatrix);
			// control points after transformed min and max values can change
			min = _mm_min_ps(min, max);
			max = _mm_max_ps(min, max);
	
			for (int i = 0; i < 6; ++i) {
				const __m128 p = MaxPointAlongNormal(min, max, frustum.m_planes[i]);
				const __m128 result = XMPlaneDotCoord(frustum.m_planes[i], p);
				if (XMVectorGetX(result) < 0.0f) goto frustum_intersection_missed;
			}
			bitset[bi] = true;
		frustum_intersection_missed:
	
			for (int ei = 0; ei < 8; ei++)
			{
				const __m128 blend = _mm_blendv_ps(min, max, edgeIndices[ei]);
				for (int j = 0; j < 4; j++)
				{
					const __m128 direction = Normalize(_mm_sub_ps(blend, planeSSE[j].position));
					if (Dot(direction, planeSSE[j].direction) < 0.1f) goto shadow_intersection_missed;
				}
			}
	
			bitset[1023ull + bi] = true;
		shadow_intersection_missed:
			count += bitset[1023ull + bi];
			++bi;
		}
	}
	// std::cout << count << std::endl;
	// uint32_t startIndex = 0;
	// int totalShadowed = 0;
	// for (auto& renderer : meshRenderers)
	// {
	// 	totalShadowed +=
	// 	renderer->CalculateCullingBitset(bitset, startIndex, Shadow::GetFrustumPlanes(), viewProjection);
	// }
	// std::cout << totalShadowed << std::endl;
	return bitset;
}

void Renderer3D::Dispose()
{
	PostProcessing::Dispose();
	Shadow::Dispose();
}

#ifndef NEDITOR
void Renderer3D::OnEditor()
{

	if (ImGui::RadioButton("Vsync", Vsync)) { Vsync = !Vsync; }

	if (ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_Bullet))
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.8f);
		ImGui::PushStyleColor(ImGuiCol_Border, HEADER_COLOR);

		ImGui::ColorEdit3("ambient Color", &cbGlobalData.ambientColor.x);
		ImGui::ColorEdit4("sun Color", &cbGlobalData.sunColor.x);
		if (ImGui::DragFloat("sun angle", &cbGlobalData.sunAngle)) Shadow::UpdateShadows();
		ImGui::DragFloat("ambientStrength", &cbGlobalData.ambientStength, 0.01f);

		ImGui::PopStyleColor();
		ImGui::PopStyleVar();
	}

	PostProcessing::OnEditor();
	GrassRenderer::OnEditor();

	tessMesh->OnEditor();
}
#endif