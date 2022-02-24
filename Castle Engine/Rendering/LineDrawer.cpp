#include "LineDrawer.hpp"
#include "Shader.hpp"
#include "spdlog/spdlog.h"
#include "Renderer3D.hpp"
#include "../Math.hpp"
#include "../DirectxBackend.hpp"

namespace LineDrawer {
	
	struct Line  {
		glm::vec3 pos, color; 
	};
	
	glm::vec3 currentColor;
	uint32_t currentVertexCount, targetVertexCount;
	Line* lines;
	DXBuffer* vertexBuffer;
	ID3D11Device* Device; ID3D11DeviceContext* DeviceContext;
	ID3D11InputLayout* inputLayout;
	Shader* shader;

	void CreateVertexBuffer(uint32_t size);
}

void LineDrawer::SetColor(const glm::vec3& color) { currentColor = color; }
void LineDrawer::SetShader() { shader->Bind(); }

void LineDrawer::Initialize(ID3D11Device* _device, ID3D11DeviceContext* _deviceContext)
{
	Device = _device; DeviceContext = _deviceContext;
	shader = new Shader("Shaders/Line.hlsl");
	currentColor = { 1, 0, 1 };

	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 , D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR"   , 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16 , D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	
	DirectxBackend::GetDevice()->CreateInputLayout(layout, 2, shader->VS_Buffer->GetBufferPointer(), shader->VS_Buffer->GetBufferSize(), &inputLayout);
	constexpr uint8_t StartSize = 64;
	lines = (Line*)malloc(sizeof(Line) * StartSize );
	memset(lines, 0, sizeof(Line) * StartSize );
	currentVertexCount = StartSize ;
	// create vertex buffer		
	DX_CREATE(D3D11_BUFFER_DESC, vertexBufferDesc);
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(Line) * StartSize ;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	DX_CHECK(Device->CreateBuffer(&vertexBufferDesc, nullptr, &vertexBuffer), "vertex buffer creation failed!");
}

void LineDrawer::CreateVertexBuffer(uint32_t size)
{
	lines = (Line*)realloc(lines, size * sizeof(Line));
	 
	DX_RELEASE(vertexBuffer);
	currentVertexCount = size;
	// create vertex buffer		
	DX_CREATE(D3D11_BUFFER_DESC, vertexBufferDesc);
		
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(Line) * size;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	DX_CREATE(D3D11_SUBRESOURCE_DATA, vinitData);
	vinitData.pSysMem = lines;
	D3D11_SUBRESOURCE_DATA* p_vinit = lines ? &vinitData : nullptr;
	DX_CHECK(
	Device->CreateBuffer(&vertexBufferDesc, p_vinit , &vertexBuffer), "vertex buffer creation failed!" );
}

void LineDrawer::DrawLine(const glm::vec3& a, const glm::vec3& b)
{
	if (targetVertexCount >= currentVertexCount) {
		CreateVertexBuffer(targetVertexCount + 2);
	}
	lines[targetVertexCount].pos = a;
	lines[targetVertexCount++].color = currentColor;
	
	lines[targetVertexCount].pos = b;
	lines[targetVertexCount++].color = currentColor;
}

void LineDrawer::DrawPlus(const glm::vec3& point)
{
	DrawLine(point - glm::vec3(0.5f, 0.0f, 0.0f), point + glm::vec3(0.5f, 0.0f, 0.0f));
	DrawLine(point - glm::vec3(0.0f, 0.5f, 0.0f), point + glm::vec3(0.0f, 0.5f, 0.0f));
	DrawLine(point - glm::vec3(0.0f, 0.0f, 0.5f), point + glm::vec3(0.0f, 0.0f, 0.5f));
}

void LineDrawer::DrawAABB(const AABB& aabb, bool active)
{
	const std::array<glm::vec3, 9> corners = aabb.GetXYZEdges();
	
	SetColor(active ? glm::vec3{ 0.0f, 0.8f, 0.0f} : glm::vec3{ 0.8f, 0.0f, 0.0f });

	for (int i = 0; i < corners.size(); ++i)
	{
		DrawPlus(corners[i]);
	}
}

// todo add other shapes
void LineDrawer::Render()
{
	static bool first = true;
	if (currentVertexCount < targetVertexCount) {
		CreateVertexBuffer(targetVertexCount + 64);
	}

	{
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	if(DeviceContext->Map(vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource) != S_OK) return;

	Line* mappedLines = (Line*)mappedResource.pData;
	memcpy(mappedLines, lines, sizeof(Line) * targetVertexCount);

	DeviceContext->Unmap(vertexBuffer, 0);
	}
	
	DeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	DeviceContext->IASetInputLayout(inputLayout);
	UINT stride = sizeof(Line), offset = 0;
	DeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	DeviceContext->Draw(targetVertexCount, 0);
	memset(lines, 0, sizeof(Line) * targetVertexCount);
	targetVertexCount = 0;
	DeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void LineDrawer::Dispose()
{
	free(lines);
	shader->Dispose(); DX_RELEASE(vertexBuffer);
}


