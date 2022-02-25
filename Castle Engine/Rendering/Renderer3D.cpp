#pragma once
#include <array>
#include <vector>
#include "../Timer.hpp"
#include "../DirectxBackend.hpp"
#include "Renderer3D.hpp"
#include "ComputeShader.hpp"
#include "Shader.hpp"
#include "Skybox.hpp"
#include "TesellatedMesh.hpp"
#include "PostProcessing.hpp"
#include "LineDrawer.hpp"
#include "../DirectxBackend.hpp"
#include "../Main/Time.hpp"
#include <array>
#include "Terrain.hpp"
#include "Grassrenderer.hpp"
#include <future>
#include <thread>
#ifndef NEDITOR
#	include "../Editor/Editor.hpp"
#endif

namespace Renderer3D
{
	DXDevice* Device; DXDeviceContext* DeviceContext;

	// post processing
	DXInputLayout* quadVertLayout;
	DXBuffer* ScreenSizeCB;
	
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

	bool Vsync = true;
	int culledMeshCount = 0;
	
	// forward declarations
	void DrawTerrain();
	void CreateBuffers();
	CullingBitset CalculateCulls();
}

RenderTexture* Renderer3D::GetPostRenderTexture() { return PostProcessing::GetPostRenderTexture(); }

__forceinline static int FormatToByteSize(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_R32G32B32A32_FLOAT: return 16;
	case DXGI_FORMAT_R32G32B32_FLOAT: return 12;
	case DXGI_FORMAT_R32G32_FLOAT: return 8;
	case DXGI_FORMAT_R32G32_SINT: return 8;
	case DXGI_FORMAT_R16G16_FLOAT: return 4;
	case DXGI_FORMAT_R32_FLOAT: return 4;
	case DXGI_FORMAT_R32_SINT: return 4;
	default: return -1;
	}
}

void Renderer3D::AddMeshRenderer(MeshRenderer* meshRenderer)
{
	meshRenderers.push_back(meshRenderer);
}

void Renderer3D::RenderMeshes()
{
	if(!cullingThread.valid()) return;

	cullingThread.wait();
	CullingBitset cullResult = cullingThread.get();
	culledMeshCount = cullResult.count();
	uint32_t startIndex = 0;

	for (auto& renderer : meshRenderers)
	{
		renderer->Draw(DeviceContext, cullResult, startIndex);
	}
}

void Renderer3D::Initialize(DXDevice* _device, DXDeviceContext* _deviceContext, unsigned int _msaaSamples, FreeCamera* camera)
{
	Device = _device; DeviceContext = _deviceContext;
	freeCamera = camera;
	
	PostProcessing::Initialize(Device, DeviceContext, MSAASamples);
	LineDrawer::Initialize(Device, DeviceContext);
	GrassRenderer::Initialize(Device, DeviceContext);
	CreateBuffers();

	PBRshader = new Shader("Shaders/First.hlsl\0");
	PBRshader->Bind();

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

	PointsAndIndices32  tessMeshCreateResult = CSCreatePlanePoints(100, 100, {0,0});
	tessMesh = new TesellatedMesh(Device, tessMeshCreateResult);
	tessMeshCreateResult.Clear();

	renderTexture = new RenderTexture(Engine::Width, Engine::Height, DirectxBackend::GetMSAASamples(), RenderTextureCreateFlags::Depth);
}

void Renderer3D::CreateBuffers()
{
	cbGlobalData.ambientColor = glm::vec3(0.8f, 0.8f, 0.65f);
	cbGlobalData.sunColor = glm::vec3(1.0f, 1.0f, 1.0f);
	cbGlobalData.ambientStength = .120f;
	cbGlobalData.sunAngle = 120;

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

#ifndef NEDITOR
void Renderer3D::OnEditor()
{
	if (ImGui::RadioButton("Vsync", Vsync)) { Vsync = !Vsync; }
	
	ImGui::Text(("CulledMeshCount: " + std::to_string(culledMeshCount)).c_str());

	if (ImGui::CollapsingHeader("Lighting"))
	{
		ImGui::ColorEdit3("ambient Color", &cbGlobalData.ambientColor.x);
		ImGui::ColorEdit4("sun Color", &cbGlobalData.sunColor.x);
		ImGui::DragFloat("sun angle", &cbGlobalData.sunAngle);
		ImGui::DragFloat("ambientStrength", &cbGlobalData.ambientStength, 0.01f);
	}

	PostProcessing::OnEditor();
	GrassRenderer::OnEditor();

	tessMesh->OnEditor();
}
#endif

// screen space quad for post processing and other stuff
void Renderer3D::RenderToQuad() {
	DeviceContext->IASetInputLayout(nullptr);
	DeviceContext->IASetIndexBuffer(nullptr, (DXGI_FORMAT)0, 0);
	DeviceContext->IASetInputLayout(nullptr);
	DeviceContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);

	DeviceContext->Draw(3, 0);
}

// do not send matrix transposed!
void Renderer3D::SetModelMatrix(const XMMATRIX& matrix)
{
	const auto MVP = matrix * freeCamera->GetViewProjection();
	cbPerObj.MVP = XMMatrixTranspose(MVP);
	cbPerObj.Model = XMMatrixTranspose(matrix);
	DeviceContext->UpdateSubresource(constantBuffer, 0, NULL, &cbPerObj, 0, 0);
	DeviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);
}

void Renderer3D::DrawTerrain()
{
	Terrain::BindShader();

	SetModelMatrix(XMMatrixIdentity());

	cbGlobalData.additionalData = Terrain::GetTextureScale();
	DeviceContext->UpdateSubresource(uniformGlobalBuffer, 0, NULL, &cbGlobalData, 0, 0);

	DeviceContext->PSSetConstantBuffers(2, 1, &uniformGlobalBuffer);
	FrustumBitset frustumResult = Terrain::Draw(*freeCamera);
	
	// 600k grass in 0.07 seconds depends on sea level framerate can increase %20
	GrassRenderer::SetShader(freeCamera->GetViewProjection());
	Terrain::DrawGrasses(*freeCamera, frustumResult);
	GrassRenderer::EndRender();

	DeviceContext->RSSetState(rasterizerState);
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

#ifndef NEDITOR
	D3D11_VIEWPORT viewPort = DirectxBackend::GetViewPort();

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
	Renderer3D::RenderMeshes();

	DeviceContext->RSSetState(WireframeRasterizerState);
	// SetModelMatrix(XMMatrixTranslation(-000, 0, -000) * XMMatrixScaling(7, 7, 7));
	// tessMesh->Render(DeviceContext, cbPerObj.MVP, freeCamera->transform.GetPosition());
	DeviceContext->RSSetState(rasterizerState);
	
	LineDrawer::SetShader();
	SetModelMatrix(XMMatrixTranslation(-000, 0, -000) * XMMatrixScaling(1, 1, 1));
	LineDrawer::Render();

	DrawTerrain();
	
	skybox->Draw(cbPerObj, DeviceContext, constantBuffer, freeCamera);

#ifndef NEDITOR
	PostProcessing::Proceed(renderTexture->textureView, renderTexture->sampler, false);
#endif
	
	DirectxBackend::SetBackBufferAsRenderTarget();

#ifdef NEDITOR
	Renderer3D::PostProcessing(renderTexture->textureView, renderTexture->sampler, true);
#endif
	// set default render buffers again
	DeviceContext->OMSetDepthStencilState(nullptr, 0);
	DeviceContext->RSSetState(rasterizerState);

	DirectxBackend::SetBackBufferAsRenderTarget();
	DeviceContext->IASetInputLayout(PBRVertLayout);

#ifndef NEDITOR
	viewPort = DirectxBackend::GetViewPort();
	DeviceContext->RSSetViewports(1, &viewPort);
#endif

	PBRshader->Bind();

#ifndef NEDITOR
	// Draw Imgui
	Editor::Render();
#endif

	//Present the backbuffer to the screen
	DirectxBackend::Present(Vsync);

	freeCamera->Update();
	cullingThread = std::async(std::launch::async, CalculateCulls);
}

// this tooks 0.23ms-0.7ms runs asyncrusly on seperate thread
CullingBitset Renderer3D::CalculateCulls() // angle culling (like frustum culling)
{
	CullingBitset bitset {};
	AABBCullData cullData {
		nullptr,
		freeCamera->transform.GetPosition(),
		freeCamera->transform.GetForward(),
		glm::cross(freeCamera->transform.GetRight(), freeCamera->transform.GetForward()),
		freeCamera->fov
	};
	
	uint32_t startIndex = 0;

	for (auto& renderer : meshRenderers)
	{
		renderer->CalculateCullingBitset(bitset, startIndex, cullData);
	}
	return bitset;
}

void Renderer3D::Dispose()
{
	PostProcessing::Dispose();
}
