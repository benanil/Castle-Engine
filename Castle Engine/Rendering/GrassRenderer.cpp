#include "GrassRenderer.hpp"
#include "../Rendering.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "Renderer3D.hpp"
#include "Terrain.hpp"
#include <array>

namespace GrassRenderer
{
	struct GrassVertex
	{
		glm::vec3 position;
		XMHALF2 uv;
		GrassVertex(const glm::vec3& _position, XMHALF2 _uv) : position(_position), uv(_uv) { };
	};

	ID3D11Device* Device;  ID3D11DeviceContext* DeviceContext;
	Shader* shader; Texture* grassTexture;
	ID3D11Buffer* vertexBuffer, *indexBuffer;
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

	std::array<GrassVertex, 4> vertices =
	{
		GrassVertex(glm::vec3(-0.0f, -0.0f, 0), XMHALF2(0.0f, 0.0f)),
		GrassVertex(glm::vec3(-0.0f, 10.0f, 0), XMHALF2(0.0f, 1.0f)),
		GrassVertex(glm::vec3(10.0f, -0.0f, 0), XMHALF2(1.0f, 0.0f)),
		GrassVertex(glm::vec3(10.0f, 10.0f, 0), XMHALF2(1.0f, 1.0f))
	};

	std::array<uint16_t, 6> indices = {
		0,1,2, 1,3,2
	};

	CSCreateVertexIndexBuffers(Device,vertices.data(), indices.data(), 
		vertices.size(), indices.size(), &vertexBuffer, &indexBuffer);

	shader = new Shader("Shaders/Grass.hlsl");
	grassTexture = new Texture("Textures/Grass.png");

	DXInputLayout* vertLayout;

	const D3D11_INPUT_ELEMENT_DESC inputInfo[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA  , 0},
		{ "TEXCOORD", 0, DXGI_FORMAT_R16G16_FLOAT   , 0, offsetof(GrassVertex, uv), D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "INSTANCE_POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1}
	};
	Device->CreateInputLayout(inputInfo, 3, shader->VS_Buffer, shader->VS_Buffer->GetBufferSize(), &inputLayout);
}
 
void GrassRenderer::SetShader()
{ 
	shader->Bind(); 
	DeviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);
	DeviceContext->IASetInputLayout(inputLayout);
	//Set the vertex buffer
	
	UINT stride = sizeof(GrassVertex), offset = 0;
	DeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	DeviceContext->RSSetState(rasterizerState);
	grassTexture->Bind(DeviceContext, 0);
}

void GrassRenderer::Render(ID3D11Buffer* instancingSRV)
{
	UINT stride = sizeof(glm::vec3), offset = 0;
	DeviceContext->IASetVertexBuffers(1, 1, &instancingSRV, &stride, &offset);
	DeviceContext->DrawIndexedInstanced(6, TERRAIN_GRASS_PER_CHUNK, 0, 0, 0);
}
