// same as LineDrawer but 2d
// immediate mode 2D line & line shape drawer
#include "Line2D.hpp"
#include "Shader.hpp"
#include "Renderer3D.hpp"
#include "../Math.hpp"
#include "../DirectxBackend.hpp"
#include "compile_time/math.hpp"
#include "../Timer.hpp"

using namespace CMath;

namespace LineDrawer2D {

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
	FreeCamera* camera;

	void CreateVertexBuffer(uint32_t size);
}

const Color32& LineDrawer2D::GetColor() { return currentColor; };
void LineDrawer2D::SetColor(const Color32& color) { currentColor = color; }
void LineDrawer2D::SetShader() { shader->Bind(); }

void LineDrawer2D::Initialize(FreeCamera* _camera)
{
	Device = DirectxBackend::GetDevice(); DeviceContext = DirectxBackend::GetDeviceContext();
	shader = new Shader("Shaders/Line2D.hlsl"); 
	currentColor = Color32(0, 255, 0); camera = _camera;

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

void LineDrawer2D::CreateVertexBuffer(uint32_t size)
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

void LineDrawer2D::DrawLine(const glm::vec2& a, const glm::vec2& b)
{
	if (a.x < -1.0f || b.x < -1.0f || a.y < -1.0f || b.y < -1.0f || 
		a.x >  1.0f || b.x >  1.0f || a.y >  1.0f || b.y >  1.0f) return;

	if (targetVertexCount >= currentVertexCount) {
		CreateVertexBuffer(targetVertexCount + 2);
	}
	lines[targetVertexCount].pos = a;
	lines[targetVertexCount++].color = currentColor;

	lines[targetVertexCount].pos = b;
	lines[targetVertexCount++].color = currentColor;
}

namespace ct = compile_time;
constexpr float thick = 0.0120f; // compile time sincos calculation
constexpr float s0 = compile_time::sin(0.0f) * thick;
constexpr float s1 = compile_time::sin(0.8f) * thick;
constexpr float s2 = compile_time::sin(1.6f) * thick;
constexpr float s3 = compile_time::sin(2.4f) * thick;
constexpr float s4 = compile_time::sin(glm::pi<float>()) * thick;
constexpr float c0 = compile_time::cos(0.0f) * thick;
constexpr float c1 = compile_time::cos(0.8f) * thick;
constexpr float c2 = compile_time::cos(1.6f) * thick;
constexpr float c3 = compile_time::cos(2.4f) * thick;
constexpr float c4 = compile_time::cos(glm::pi<float>()) * thick;

void LineDrawer2D::DrawThickLine(const glm::vec3& a, const glm::vec3& b, float scale)
{
	float SinCosArray[] = { s0, s1, s2, s3, s4, c0, c1, c2, c3, c4  };

	_mm_store_ps(SinCosArray, _mm_mul_ps(_mm_load_ps(SinCosArray), _mm_set_ps1(scale)));
	_mm_store_ps(SinCosArray + 4, _mm_mul_ps(_mm_load_ps(SinCosArray + 4), _mm_set_ps1(scale)));
	
	SinCosArray[8] *= scale; SinCosArray[9] *= scale;

	if (fabs(glm::normalize(a - b).y) > 0.5f) { // line is vertical
		DrawLine(camera->WorldToNDC(a + glm::vec3(SinCosArray[0], 0, SinCosArray[5])), camera->WorldToNDC(b + glm::vec3(SinCosArray[0], 0, SinCosArray[5])));
		DrawLine(camera->WorldToNDC(a + glm::vec3(SinCosArray[1], 0, SinCosArray[6])), camera->WorldToNDC(b + glm::vec3(SinCosArray[1], 0, SinCosArray[6])));
		DrawLine(camera->WorldToNDC(a + glm::vec3(SinCosArray[2], 0, SinCosArray[7])), camera->WorldToNDC(b + glm::vec3(SinCosArray[2], 0, SinCosArray[7])));
		DrawLine(camera->WorldToNDC(a + glm::vec3(SinCosArray[3], 0, SinCosArray[8])), camera->WorldToNDC(b + glm::vec3(SinCosArray[3], 0, SinCosArray[8])));
		DrawLine(camera->WorldToNDC(a + glm::vec3(SinCosArray[4], 0, SinCosArray[9])), camera->WorldToNDC(b + glm::vec3(SinCosArray[4], 0, SinCosArray[9])));
	}
	else { // line is horizontal
		DrawLine(camera->WorldToNDC(a + glm::vec3(SinCosArray[0], SinCosArray[5], 0)), camera->WorldToNDC(b + glm::vec3(SinCosArray[0], SinCosArray[5], 0)));
		DrawLine(camera->WorldToNDC(a + glm::vec3(SinCosArray[1], SinCosArray[6], 0)), camera->WorldToNDC(b + glm::vec3(SinCosArray[1], SinCosArray[6], 0)));
		DrawLine(camera->WorldToNDC(a + glm::vec3(SinCosArray[2], SinCosArray[7], 0)), camera->WorldToNDC(b + glm::vec3(SinCosArray[2], SinCosArray[7], 0)));
		DrawLine(camera->WorldToNDC(a + glm::vec3(SinCosArray[3], SinCosArray[8], 0)), camera->WorldToNDC(b + glm::vec3(SinCosArray[3], SinCosArray[8], 0)));
		DrawLine(camera->WorldToNDC(a + glm::vec3(SinCosArray[4], SinCosArray[9], 0)), camera->WorldToNDC(b + glm::vec3(SinCosArray[4], SinCosArray[9], 0)));
	}
}

void LineDrawer2D::DrawThickArrow(const glm::vec3& a, const glm::vec3& b, float scale)
{
	DrawThickLine(a, b, scale);
	glm::vec3 dir = glm::normalize(a - b);

	float dist = glm::distance(a, b);
	float percent = dist * 0.128f;
	glm::vec3 temp = b + (dir * percent);

	if (fabs(dir.y) > 0.5f) { // line is vertical
		DrawThickLine(temp - glm::vec3(percent, +percent, 0.0f), b, scale);
		DrawThickLine(temp - glm::vec3(0.0f, +percent, percent), b, scale);
	}
	else {
		DrawThickLine(temp - glm::vec3(0.0f, percent, 0.0f) , b, scale);
		DrawThickLine(temp - glm::vec3(0.0f, -percent, 0.0f), b, scale);
	}
}

void LineDrawer2D::DrawLine(const glm::vec3& a, const glm::vec3& b)
{
	DrawLine(camera->WorldToNDC(a), camera->WorldToNDC(b));
}

void LineDrawer2D::DrawCircale(const glm::vec3& center, float circumferance, int segments)
{
	DrawCircale(camera->WorldToNDC(center), circumferance, segments);
}

void LineDrawer2D::DrawCircale(const glm::vec2& center, float circumferance, int segments)
{
	float percent = glm::two_pi<float>() / segments;
	for (int i = 0; i < segments; ++i)
	{
		int i1 = i + 1;
		float aspectRatioEffect = 1.0f / camera->aspectRatio;
		DrawLine(center + glm::vec2(glm::sin(percent * i) * circumferance * aspectRatioEffect, glm::cos(percent * i) * circumferance),
			center + glm::vec2(glm::sin(percent * i1) * circumferance * aspectRatioEffect, glm::cos(percent * i1) * circumferance));
	}
}

void LineDrawer2D::DrawPlus(const glm::vec2& point)
{
	DrawLine(point - glm::vec2(0.5f, 0.0f), point + glm::vec2(0.5f, 0.0f));
	DrawLine(point - glm::vec2(0.0f, 0.5f), point + glm::vec2(0.0f, 0.5f));
	DrawLine(point - glm::vec2(0.0f, 0.0f), point + glm::vec2(0.0f, 0.0f));
}

void LineDrawer2D::DrawCube(const glm::vec2& position, const glm::vec2& scale)
{
	glm::vec2 min = glm::vec2(position.x - (scale.x * 0.5f), position.x - (scale.x * 0.5f));
	glm::vec2 max = glm::vec2(position.x + (scale.x * 0.5f), position.y + (scale.y * 0.5f));
	
	min.x /= camera->aspectRatio; 
	max.x /= camera->aspectRatio;

	DrawLine(glm::vec2(min.x, min.y), glm::vec2(min.x, max.y));
	DrawLine(glm::vec2(min.x, max.y), glm::vec2(max.x, max.y));
	DrawLine(glm::vec2(max.x, max.y), glm::vec2(max.x, min.y));
	DrawLine(glm::vec2(max.x, min.y), glm::vec2(min.x, min.y));
}

// todo add other shapes
void LineDrawer2D::Render()
{
	LineDrawer2D::SetShader();
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

void LineDrawer2D::Dispose()
{
	free(lines);
	shader->Dispose(); DX_RELEASE(vertexBuffer);
}


