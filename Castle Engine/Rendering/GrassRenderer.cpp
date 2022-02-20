#include "GrassRenderer.hpp"
#include "../Rendering.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "Renderer3D.hpp"
#include "Terrain.hpp"
#include <array>
#include "../Main/Time.hpp"

#ifndef NEDITOR
#	include "../Editor/Editor.hpp"
#endif

namespace GrassRenderer
{
	ID3D11Device* Device;  ID3D11DeviceContext* DeviceContext;
	Shader* shader; Texture* grassTexture;
	ID3D11RasterizerState* rasterizerState;
	ID3D11InputLayout* inputLayout;
	ID3D11Buffer* vertexBuffer, *indexBuffer;
	ID3D11Buffer* cbuffer;
	ID3D11BlendState* blendState;
	struct Cbuffer {
		XMMATRIX viewProjection;
		float time;    
		glm::vec3 color = { 0.720f, 0.916f, 0.263f };
		glm::vec3 windDir = { 0.2, 0, 0.8 }; 
		float windSpeed = 1.0f;
	} cbufferData;
}

#ifndef NEDITOR
void GrassRenderer::OnEditor()
{
	if (ImGui::CollapsingHeader("Grass Renderer"))
	{
		ImGui::ColorEdit3("Color", &cbufferData.color.x);
		ImGui::DragFloat3("Wind Direction", &cbufferData.windDir.x);
		ImGui::DragFloat("Wind Speed", &cbufferData.windSpeed);
	}
}
#endif

void GrassRenderer::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	Device = device; DeviceContext = deviceContext;

	DXCreateConstantBuffer(Device, cbuffer, &cbufferData);

	D3D11_BLEND_DESC blendDescription;
	ZeroMemory(&blendDescription, sizeof(D3D11_BLEND_DESC));
	blendDescription.RenderTarget[0].BlendEnable = TRUE;
	blendDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
	blendDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDescription.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	Device->CreateBlendState(&blendDescription, &blendState);

	// Create Rasterizer 
	DX_CREATE(D3D11_RASTERIZER_DESC, rasterizerDesc);
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_NONE ;
	rasterizerDesc.MultisampleEnable = false;

	DX_CHECK(
	Device->CreateRasterizerState(&rasterizerDesc, &rasterizerState), "rasterizer creation failed");

	shader = new Shader("Shaders/Grass.hlsl");
	grassTexture = new Texture("Textures/Grass.png");

	std::array<GrassVertex, 4> vertices;
	memset(vertices.data(), 0, sizeof(GrassVertex) * 4);
	vertices[1].position.y = 14.0f;
	vertices[2].position.x = 13.0f;
	vertices[3].position.x = 13.0f;
	vertices[3].position.y = 14.0f;
	vertices[0].uv = XMHALF2(0.0f, 1.1f);
	vertices[1].uv = XMHALF2(0.0f, 0.0f);
	vertices[2].uv = XMHALF2(1.1f, 1.1f);
	vertices[3].uv = XMHALF2(1.1f, 0.0f);
	
	DX_CREATE(D3D11_BUFFER_DESC, vertexBufferDesc);
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(GrassVertex) * vertices.size();
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	DX_CREATE(D3D11_SUBRESOURCE_DATA, vinitData);
	vinitData.pSysMem = vertices.data();
	if (FAILED(device->CreateBuffer(&vertexBufferDesc, &vinitData, &vertexBuffer))) {
		assert(0, "Constant Buffer Creation Failed!");
	}

	std::array<uint32_t, 6> indices =  {
		0, 1, 2, 1, 3, 2
	};

	DX_CREATE(D3D11_BUFFER_DESC, indexBufferDesc);
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(uint32_t) * 6;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	vinitData.pSysMem = indices.data();

	if (FAILED(device->CreateBuffer(&indexBufferDesc, &vinitData, &indexBuffer))) {
		assert(0, "Constant Buffer Creation Failed!");
	}

	D3D11_INPUT_ELEMENT_DESC inputInfo[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,   0, D3D11_INPUT_PER_VERTEX_DATA  , 0},
		{ "TEXCOORD", 0, DXGI_FORMAT_R16G16_FLOAT   , 0,  12, D3D11_INPUT_PER_VERTEX_DATA  , 0}
	};
	DX_CHECK(
	Device->CreateInputLayout(inputInfo, 2, shader->VS_Buffer->GetBufferPointer(), shader->VS_Buffer->GetBufferSize(), &inputLayout), "grass input layout failed");
}
namespace { ID3D11BlendState* oldBlendState; }

void GrassRenderer::SetShader(const XMMATRIX& view, const XMMATRIX& projection)
{ 
	cbufferData.viewProjection = XMMatrixTranspose(view * projection); // this is not mvp we need view projection here
	cbufferData.time = Time::GetTimeSinceStartup();
	DeviceContext->UpdateSubresource(cbuffer, 0, NULL, &cbufferData, 0, 0);
	DeviceContext->VSSetConstantBuffers(0, 1, &cbuffer);

	shader->Bind(); 
	DeviceContext->IASetInputLayout(inputLayout);	
	DeviceContext->RSSetState(rasterizerState);
	grassTexture->Bind(DeviceContext, 0);
	DeviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	UINT stride = sizeof(GrassVertex), offset = 0;
	DeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	
	DeviceContext->OMGetBlendState(&oldBlendState, nullptr, nullptr);
	DeviceContext->OMSetBlendState(blendState, 0, 0xffffffff);
}

void GrassRenderer::Render(const GrassGroup& grassGroup)
{
	UINT stride = sizeof(glm::vec3), offset = 0;
	DeviceContext->VSSetShaderResources(0, 1, &grassGroup.srv);
	DeviceContext->DrawIndexedInstanced(6, TERRAIN_GRASS_PER_CHUNK, 0, 0, 0);
}

void GrassRenderer::EndRender()
{
	DeviceContext->OMSetBlendState(oldBlendState, 0, 0xffffffff);
}
