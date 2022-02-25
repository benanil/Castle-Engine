// immediate mode line & line shape drawer

#include "LineDrawer.hpp"
#include "Shader.hpp"
#include "spdlog/spdlog.h"
#include "Renderer3D.hpp"
#include "../Math.hpp"
#include "../DirectxBackend.hpp"
#include <vector>
#include <array>

namespace LineDrawer {
	
	struct Line  {
		glm::vec4 pos;
		glm::vec3 color;
	};
	
	glm::vec3 currentColor;
	XMMATRIX currentMatrix;
	uint32_t currentVertexCount, targetVertexCount;
	Line* lines;
	
	DXBuffer* vertexBuffer;
	ID3D11Device* Device; ID3D11DeviceContext* DeviceContext;
	ID3D11InputLayout* inputLayout;
	Shader* shader;

	void CreateVertexBuffer(uint32_t size);
	void DrawTransformedLine(const glm::vec4& a, const glm::vec4& b); // for not using matrix
	void DrawCube(const std::array<glm::vec4, 8>& corners);
	glm::vec4 TransformVector(const glm::vec3& pos);
}

void LineDrawer::SetColor(const glm::vec3& color) { currentColor = color; }
void LineDrawer::SetMatrix(const XMMATRIX& matrix) { currentMatrix = matrix; }
void LineDrawer::SetShader() { shader->Bind(); }

void LineDrawer::Initialize(ID3D11Device* _device, ID3D11DeviceContext* _deviceContext)
{
	Device = _device; DeviceContext = _deviceContext;
	currentMatrix = XMMatrixIdentity();
	shader = new Shader("Shaders/Line.hlsl");
	currentColor = { 1, 0, 1 };

	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0 , D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR"   , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(glm::vec4), D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	
	DirectxBackend::GetDevice()->CreateInputLayout(layout, 2, shader->VS_Buffer->GetBufferPointer(), shader->VS_Buffer->GetBufferSize(), &inputLayout);
	constexpr uint8_t StartSize = 64;
	lines = (Line*)malloc(sizeof(Line) * StartSize );
	memset(lines, 0, sizeof(Line) * StartSize);
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

glm::vec4 LineDrawer::TransformVector(const glm::vec3& pos)
{
	glm::vec4 temp;
	_mm_store_ps(&temp.x, XMVector3Transform(_mm_load_ps(&pos.x), currentMatrix));
	temp.w = 1;
	return temp;
}

void LineDrawer::DrawTransformedLine(const glm::vec4& a, const glm::vec4& b)
{
	if (targetVertexCount >= currentVertexCount) {
		CreateVertexBuffer(targetVertexCount + 2);
	}
	lines[targetVertexCount].pos = a;
	lines[targetVertexCount++].color = currentColor;

	lines[targetVertexCount].pos = b;
	lines[targetVertexCount++].color = currentColor;
}

void LineDrawer::DrawLine(const glm::vec3& a, const glm::vec3& b)
{
	DrawTransformedLine(TransformVector(a), TransformVector(b));
}

void LineDrawer::DrawCircale(const glm::vec3& center, float circumferance, int segments)
{
	float percent = glm::two_pi<float>() / segments;
	for (int i = 0; i < segments; ++i)
	{
		int i1 = i + 1;
		DrawLine(center + glm::vec3(glm::sin(percent * i) * circumferance, 0, glm::cos(percent * i) * circumferance),
				 center + glm::vec3(glm::sin(percent * i1) * circumferance, 0, glm::cos(percent * i1) * circumferance));

		DrawLine(center + glm::vec3(0, glm::sin(percent * i) * circumferance, glm::cos(percent * i) * circumferance),
				 center + glm::vec3(0, glm::sin(percent * i1) * circumferance, glm::cos(percent * i1) * circumferance));

		DrawLine(center + glm::vec3(glm::sin(percent * i) * circumferance, glm::cos(percent * i) * circumferance, 0.0f),
			 	 center + glm::vec3(glm::sin(percent * i1) * circumferance, glm::cos(percent * i1) * circumferance, 0.0f));
	}
}

void LineDrawer::DrawPlus(const glm::vec3& point)
{
	DrawLine(point - glm::vec3(0.5f, 0.0f, 0.0f), point + glm::vec3(0.5f, 0.0f, 0.0f));
	DrawLine(point - glm::vec3(0.0f, 0.5f, 0.0f), point + glm::vec3(0.0f, 0.5f, 0.0f));
	DrawLine(point - glm::vec3(0.0f, 0.0f, 0.5f), point + glm::vec3(0.0f, 0.0f, 0.5f));
}

void LineDrawer::DrawCube(const std::array<glm::vec4, 8>& corners)
{
	// first 4 corner
	DrawTransformedLine(corners[0], corners[6]);
	DrawTransformedLine(corners[6], corners[2]);
	DrawTransformedLine(corners[2], corners[4]);
	DrawTransformedLine(corners[4], corners[0]);
	// second 4 corner
	DrawTransformedLine(corners[1], corners[7]);
	DrawTransformedLine(corners[7], corners[3]);
	DrawTransformedLine(corners[3], corners[5]);
	DrawTransformedLine(corners[5], corners[1]);
	// connect corners
	DrawTransformedLine(corners[0], corners[3]);
	DrawTransformedLine(corners[6], corners[5]);
	DrawTransformedLine(corners[2], corners[1]);
	DrawTransformedLine(corners[4], corners[7]);
}

void LineDrawer::DrawAABB(const AABB& aabb, bool active)
{
	const auto corners = aabb.GetXYZEdges4(currentMatrix); // returns transformed cube corners
	SetColor(active ? glm::vec3{ 0.0f, 0.8f, 0.0f} : glm::vec3{ 0.8f, 0.0f, 0.0f });
	DrawCube(corners);
}

void LineDrawer::DrawCube(const glm::vec3& position, const glm::vec3& scale)
{
	glm::vec4 min = glm::vec4(position.x - (scale.x * 0.5f), position.x - (scale.x * 0.5f), position.x - (scale.x * 0.5f), 1);
	glm::vec4 max = glm::vec4(position.x + (scale.x * 0.5f), position.y + (scale.y * 0.5f), position.z + (scale.z * 0.5f), 1);
	
	std::array<glm::vec4, 8> corners =
	{
		glm::vec4(min.x, min.y, min.z, 1.0f),
		glm::vec4(max.x, max.y, max.z, 1.0f), //
		glm::vec4(max.x, max.y, min.z, 1.0f),
		glm::vec4(min.x, min.y, max.z, 1.0f), //
		glm::vec4(min.x, max.y, min.z, 1.0f),
		glm::vec4(max.x, min.y, max.z, 1.0f), //
		glm::vec4(max.x, min.y, min.z, 1.0f),
		glm::vec4(min.x, max.y, max.z, 1.0f)  //
	};
	
	for (auto& corner : corners) _mm_store_ps(&corner.x, XMVector3Transform(_mm_load_ps(&corner.x), currentMatrix));
	
	DrawCube(corners);
}

// todo add other shapes
void LineDrawer::Render()
{
	static bool first = true;
	if (currentVertexCount < targetVertexCount) {
		CreateVertexBuffer(targetVertexCount + 64);
	}

	DrawCircale(glm::vec3(-40, 5, 0),  5);
	SetColor(glm::vec3(.5f, .5f, 0.2f));
	DrawCircale(glm::vec3(-40, 5, 0), 15);
	SetColor(glm::vec3(0.0f, 0.0f, 1.0f));
	SetMatrix(XMMatrixIdentity() * XMMatrixRotationRollPitchYaw(92.3f, 980.9f, 977.2f) * XMMatrixTranslation(-50.0f, 20.0f, 0.0f));
	DrawCube(glm::vec3(0, 0, 0), glm::vec3(50, 30, 50));

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
	currentColor = { 0, 1, 0 };
	SetMatrix(XMMatrixIdentity());
}

void LineDrawer::Dispose()
{
	free(lines);
	shader->Dispose(); DX_RELEASE(vertexBuffer);
}


