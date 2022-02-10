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
#include "../DirectxBackend.hpp"
#include "../Main/Time.hpp"
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

	DXBuffer* quadVertexBuffer, * quadIndexBuffer;
	
	DrawIndexedInfo drawInfo;
	unsigned int MSAASamples;
	
	DXInputLayout* PBRVertLayout;
	cbGlobal cbGlobalData;
	cbPerObject cbPerObj;

	DXBuffer* constantBuffer;
	DXBuffer* uniformGlobalBuffer;

	RenderTexture* renderTexture;
	Skybox* skybox;
	Shader* PBRshader;
	ID3D11RasterizerState* rasterizerState;
	ID3D11RasterizerState* WireframeRasterizerState;
	TesellatedMesh* tessMesh;
	FreeCamera* freeCamera;

	bool Vsync = true;

	PostProcessing* postProcessing;

	// forward declarations
	void DrawTerrain();
	void CreateBuffers();
	void PrepareScreenSpaceQuad();
	
}

RenderTexture* Renderer3D::GetPostRenderTexture() { return postProcessing->GetPostRenderTexture(); }

__forceinline static int FormatToByteSize(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_R32G32B32A32_FLOAT: return 16;
	case DXGI_FORMAT_R32G32B32_FLOAT: return 12;
	case DXGI_FORMAT_R32G32_FLOAT: return 8;
	case DXGI_FORMAT_R32G32_SINT: return 8;
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
	for (auto& renderer : meshRenderers)
	{
		renderer->Draw(DeviceContext);
	}
}

void Renderer3D::SetModelMatrix(const XMMATRIX& matrix)
{
	const auto MVP = matrix * freeCamera->ViewProjection;
	cbPerObj.MVP = XMMatrixTranspose(MVP);
	cbPerObj.Model = XMMatrixTranspose(matrix);
	DeviceContext->UpdateSubresource(constantBuffer, 0, NULL, &cbPerObj, 0, 0);
	DeviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);
}

DXInputLayout* Renderer3D::CreateVertexInputLayout(std::vector<InputLayoutCreateInfo> infos, DXBlob* VS_Buffer)
{
	DXInputLayout* vertLayout;

	D3D11_INPUT_ELEMENT_DESC* layout = (D3D11_INPUT_ELEMENT_DESC*)malloc(infos.size() * sizeof(D3D11_INPUT_ELEMENT_DESC));
	unsigned int byteIndex = 0;
	for (int i = 0; i < infos.size(); ++i)
	{
		layout[i] = { infos[i].name, 0, infos[i].format, 0, byteIndex, D3D11_INPUT_PER_VERTEX_DATA, 0 };
		byteIndex += FormatToByteSize(infos[i].format);
	}
	auto device = DirectxBackend::GetDevice();
	device->CreateInputLayout(layout, infos.size(), VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), &vertLayout);
	delete layout;
	return vertLayout;
}

void Renderer3D::PrepareScreenSpaceQuad()
{
	std::array<ScreenQuadVertex, 4> vertices;
	vertices[0] = { glm::vec2(-1.0f, -1.0f), glm::vec2(0.0f, 0.0f) };
	vertices[1] = { glm::vec2(-1.0f,  1.0f), glm::vec2(0.0f, 1.0f) };
	vertices[2] = { glm::vec2( 1.0f, -1.0f), glm::vec2(1.0f, 0.0f) };
	vertices[3] = { glm::vec2( 1.0f,  1.0f), glm::vec2(1.0f, 1.0f) };

	std::array<uint16_t, 6> indices = {
		0,1,2, 1,3,2
	};

	CSCreateVertexIndexBuffers<ScreenQuadVertex, uint16_t>(
		Device, vertices.data(), indices.data(),
		vertices.size(), indices.size(), &quadVertexBuffer, &quadIndexBuffer);

	std::vector<InputLayoutCreateInfo> vertexLayoutInfos =
	{
		{"POSITION", DXGI_FORMAT_R32G32_FLOAT}, {"TEXCOORD", DXGI_FORMAT_R32G32_FLOAT}
	};

	quadVertLayout = CreateVertexInputLayout(vertexLayoutInfos, postProcessing->GetShader()->VS_Buffer);

	drawInfo =
	{
		DeviceContext, quadVertLayout ,
		quadVertexBuffer, quadIndexBuffer, 6
	};	
}

void Renderer3D::Initialize(DXDevice* _device, DXDeviceContext* _deviceContext, unsigned int _msaaSamples, FreeCamera* camera)
{
	Device = _device; DeviceContext = _deviceContext;
	freeCamera = camera;

	postProcessing = new PostProcessing(Device, DeviceContext, MSAASamples);
	PrepareScreenSpaceQuad();
	CreateBuffers();

	PBRshader = new Shader("Shaders/First.hlsl\0");
	PBRshader->Bind();

	//Create the Input Layout & Set the Input Layout
	std::vector<InputLayoutCreateInfo> vertexLayoutInfos =
	{
		{ "POSITION", DXGI_FORMAT_R32G32B32_FLOAT},
		{ "NORMAL"  , DXGI_FORMAT_R32G32B32_FLOAT},
		{ "TEXCOORD", DXGI_FORMAT_R32G32_FLOAT   },
		{ "TANGENT" , DXGI_FORMAT_R32G32B32_FLOAT}
	};
	PBRVertLayout = Renderer3D::CreateVertexInputLayout(vertexLayoutInfos, PBRshader->VS_Buffer);
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
	std::cout << "Terrain created" << std::endl;
	TerrainCreateResult tessMeshCreateResult = Terrain::CreateSingleChunk();
	tessMesh = new TesellatedMesh(Device, tessMeshCreateResult.vertices, tessMeshCreateResult.indices);
	tessMeshCreateResult.Dispose();

	renderTexture = new RenderTexture(Engine::Width, Engine::Height, DirectxBackend::GetMSAASamples(), RenderTextureCreateFlags::Depth);
}

void Renderer3D::CreateBuffers()
{
	cbGlobalData.ambientColor = glm::vec3(0.8f, 0.8f, 0.65f);
	cbGlobalData.sunColor = glm::vec3(1.0f, 1.0f, 1.0f);
	cbGlobalData.ambientStength = .13f;
	cbGlobalData.sunAngle = 120;

	cbPerObj.MVP = XMMatrixTranspose(freeCamera->ViewProjection);
	cbPerObj.Model = XMMatrixIdentity();

	DXCreateConstantBuffer<cbGlobal>(Device, uniformGlobalBuffer, &cbGlobalData);
	DXCreateConstantBuffer<cbPerObject>(Device, constantBuffer, &cbPerObj);
	
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
		postProcessing->WindowScaleEvent((int)w, (int)h);
		Editor::GameViewWindow::GetData().texture = Renderer3D::GetPostRenderTexture()->textureView;
	}));
#endif
}

#ifndef NEDITOR
void Renderer3D::OnEditor()
{
	if (ImGui::RadioButton("Vsync", Vsync)) { Vsync = !Vsync; }
	
	if (ImGui::CollapsingHeader("Lighting"))
	{
		ImGui::ColorEdit3("ambient Color", &cbGlobalData.ambientColor.x);
		ImGui::ColorEdit4("sun Color", &cbGlobalData.sunColor.x);
		ImGui::DragFloat("sun angle", &cbGlobalData.sunAngle);
		ImGui::DragFloat("ambientStrength", &cbGlobalData.ambientStength, 0.01f);
	}

	postProcessing->OnEditor();
	
	tessMesh->OnEditor();
}
#endif

void Renderer3D::RenderToQuad() {
	DrawIndexed16<ScreenQuadVertex>(&drawInfo);
}

void Renderer3D::DrawTerrain()
{
	Terrain::BindShader();

	cbPerObj.MVP = XMMatrixTranspose(XMMatrixTranslation(-000, 0, -1100) * XMMatrixScaling(7, 7, 7) * freeCamera->ViewProjection);

	DeviceContext->UpdateSubresource(constantBuffer, 0, NULL, &cbPerObj, 0, 0);
	DeviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);

	cbGlobalData.additionalData = Terrain::GetTextureScale();
	DeviceContext->UpdateSubresource(uniformGlobalBuffer, 0, NULL, &cbGlobalData, 0, 0);

	DeviceContext->PSSetConstantBuffers(2, 1, &uniformGlobalBuffer);
	DeviceContext->RSSetState(rasterizerState);

	Terrain::Draw();
}

void Renderer3D::DrawScene()
{
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

	skybox->Draw(cbPerObj, DeviceContext, constantBuffer, freeCamera);

	DrawTerrain();

	DeviceContext->RSSetState(WireframeRasterizerState);
	cbPerObj.MVP = XMMatrixTranspose(XMMatrixTranslation(0, 0, 0) * XMMatrixScaling(7, 7, 7) * freeCamera->ViewProjection);
	tessMesh->Render(DeviceContext, constantBuffer, cbPerObj.MVP, freeCamera->transform.GetPosition());
	DeviceContext->RSSetState(rasterizerState);

#ifndef NEDITOR
	postProcessing->Proceed(renderTexture->textureView, renderTexture->sampler, false);
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
}

void Renderer3D::Dispose()
{
	postProcessing->Dispose();
}
