#include "GrassRenderer.hpp"
#include "../Rendering.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "Renderer3D.hpp"
#include "Terrain.hpp"
#include <array>

namespace GrassRenderer
{
	ID3D11Device* Device;  ID3D11DeviceContext* DeviceContext;
	Shader* shader; Texture* grassTexture;
	ID3D11RasterizerState* rasterizerState;
	ID3D11InputLayout* inputLayout;
}

void GrassRenderer::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	Device = device; DeviceContext = deviceContext;

	// Create Rasterizer 
	DX_CREATE(D3D11_RASTERIZER_DESC, rasterizerDesc);
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_NONE ;
	rasterizerDesc.MultisampleEnable = false;

	DX_CHECK(
	Device->CreateRasterizerState(&rasterizerDesc, &rasterizerState), "rasterizer creation failed");

	shader = new Shader("Shaders/Grass.hlsl");
	grassTexture = new Texture("Textures/Grass.png");

	DXInputLayout* vertLayout;

	D3D11_INPUT_ELEMENT_DESC inputInfo[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT   , 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	Device->CreateInputLayout(inputInfo, 2, shader->VS_Buffer->GetBufferPointer(), shader->VS_Buffer->GetBufferSize(), &inputLayout);
}
 
void GrassRenderer::SetShader()
{ 
	shader->Bind(); 
	DeviceContext->IASetInputLayout(inputLayout);	
	DeviceContext->RSSetState(rasterizerState);
	grassTexture->Bind(DeviceContext, 0);
}

void GrassRenderer::Render(const GrassGroup& grassGroup)
{
	DeviceContext->IASetIndexBuffer(grassGroup.indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	UINT stride = sizeof(GrassVertex), offset = 0;
	DeviceContext->IASetVertexBuffers(0, 1, &grassGroup.vertexBuffer, &stride, &offset);
	DeviceContext->DrawIndexed(TERRAIN_GRASS_PER_CHUNK * 6, 0, 0);
}
