// same as LineDrawer but 2d
// immediate mode 2D line & line shape drawer
#include "Line2D.hpp"
#include "Line2D.hpp"
#include "Shader.hpp"
#include "Renderer3D.hpp"
#include "../Math.hpp"
#include "../DirectxBackend.hpp"

namespace Line2D {

	struct LineVertex {
		glm::vec2 pos;
		Color32 color;
	};

	Color32 currentColor;
	uint32_t currentVertexCount, targetVertexCount;
	LineVertex* lines;

	DXBuffer* vertexBuffer;
	ID3D11Device* Device; ID3D11DeviceContext* DeviceContext;
	ID3D11InputLayout* inputLayout;
	Shader* shader;

	void CreateVertexBuffer(uint32_t size);
}

const Color32& Line2D::GetColor() { return currentColor; };
void Line2D::SetColor(const Color32& color) { currentColor = color; }
void Line2D::SetShader() { shader->Bind(); }

void Line2D::Initialize(ID3D11Device* _device, ID3D11DeviceContext* _deviceContext)
{
	Device = _device; DeviceContext = _deviceContext;
	shader = new Shader("Shaders/Line2D.hlsl");
	currentColor = Color32(0, 255, 0);

	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0 , D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR"   , 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, sizeof(glm::vec2), D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	DirectxBackend::GetDevice()->CreateInputLayout(layout, 2, shader->VS_Buffer->GetBufferPointer(), shader->VS_Buffer->GetBufferSize(), &inputLayout);
	constexpr uint8_t StartSize = 32;
	lines = (LineVertex*)malloc(sizeof(LineVertex) * StartSize);
	memset(lines, 0, sizeof(LineVertex) * StartSize);
	currentVertexCount = StartSize;
	// create vertex buffer		
	DX_CREATE(D3D11_BUFFER_DESC, vertexBufferDesc);
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(LineVertex) * StartSize;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	DX_CHECK(Device->CreateBuffer(&vertexBufferDesc, nullptr, &vertexBuffer), "vertex buffer creation failed!");
}

void Line2D::CreateVertexBuffer(uint32_t size)
{
	lines = (LineVertex*)realloc(lines, size * sizeof(LineVertex));

	DX_RELEASE(vertexBuffer);
	currentVertexCount = size;
	// create vertex buffer		
	DX_CREATE(D3D11_BUFFER_DESC, vertexBufferDesc);

	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(LineVertex) * size;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	DX_CREATE(D3D11_SUBRESOURCE_DATA, vinitData);
	vinitData.pSysMem = lines;
	D3D11_SUBRESOURCE_DATA* p_vinit = lines ? &vinitData : nullptr;
	DX_CHECK(
		Device->CreateBuffer(&vertexBufferDesc, p_vinit, &vertexBuffer), "vertex buffer creation failed!");
}

void Line2D::DrawLine(const glm::vec2& a, const glm::vec2& b)
{
	if (targetVertexCount >= currentVertexCount) {
		CreateVertexBuffer(targetVertexCount + 2);
	}
	lines[targetVertexCount].pos = a;
	lines[targetVertexCount++].color = currentColor;

	lines[targetVertexCount].pos = b;
	lines[targetVertexCount++].color = currentColor;
}

void Line2D::DrawCircale(const glm::vec2& center, float circumferance, int segments)
{
	float percent = glm::two_pi<float>() / segments;
	for (int i = 0; i < segments; ++i)
	{
		int i1 = i + 1;
		DrawLine(center + glm::vec2(glm::sin(percent * i) * circumferance, glm::cos(percent * i) * circumferance),
			center + glm::vec2(glm::sin(percent * i1) * circumferance, glm::cos(percent * i1) * circumferance));
	}
}

void Line2D::DrawPlus(const glm::vec2& point)
{
	DrawLine(point - glm::vec2(0.5f, 0.0f), point + glm::vec2(0.5f, 0.0f));
	DrawLine(point - glm::vec2(0.0f, 0.5f), point + glm::vec2(0.0f, 0.5f));
	DrawLine(point - glm::vec2(0.0f, 0.0f), point + glm::vec2(0.0f, 0.0f));
}

void Line2D::DrawCube(const glm::vec2& position, const glm::vec2& scale)
{
	glm::vec2 min = glm::vec2(position.x - (scale.x * 0.5f), position.x - (scale.x * 0.5f));
	glm::vec2 max = glm::vec2(position.x + (scale.x * 0.5f), position.y + (scale.y * 0.5f));
	
	DrawLine(glm::vec2(min.x, min.y), glm::vec2(min.x, max.y));
	DrawLine(glm::vec2(min.x, max.y), glm::vec2(max.x, max.y));
	DrawLine(glm::vec2(max.x, max.y), glm::vec2(max.x, min.y));
	DrawLine(glm::vec2(max.x, min.y), glm::vec2(min.x, min.y));
}

// todo add other shapes
void Line2D::Render()
{
	static bool first = true;
	if (currentVertexCount < targetVertexCount) {
		CreateVertexBuffer(targetVertexCount + 64);
	}
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;

		if (DeviceContext->Map(vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource) != S_OK) return;

		LineVertex* mappedLines = (LineVertex*)mappedResource.pData;
		memcpy(mappedLines, lines, sizeof(LineVertex) * targetVertexCount);

		DeviceContext->Unmap(vertexBuffer, 0);
	}

	DeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	DeviceContext->IASetInputLayout(inputLayout);
	UINT stride = sizeof(LineVertex), offset = 0;
	DeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	DeviceContext->Draw(targetVertexCount, 0);
	memset(lines, 0, sizeof(LineVertex) * targetVertexCount);
	targetVertexCount = 0;
	DeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	currentColor = { 0, 255, 0, 255 };
}

void Line2D::Dispose()
{
	free(lines);
	shader->Dispose(); DX_RELEASE(vertexBuffer);
}


