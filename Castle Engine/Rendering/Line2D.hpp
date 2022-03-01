#pragma once
#include "../Math.hpp"
#include "../Rendering.hpp"

namespace Line2D
{
	// draws a line between a and b
	void DrawLine(const glm::vec2& a, const glm::vec2& b);
	void SetColor(const Color32& color);
	const Color32& GetColor();
	void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	void DrawCircale(const glm::vec2& center, float circumferance, int segments = 20);
	void DrawCube(const glm::vec2& position, const glm::vec2& scale);
	void DrawPlus(const glm::vec2& point);
	void SetShader();
	void Dispose();
	void Render();
}