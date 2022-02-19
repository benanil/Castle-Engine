#pragma once
#include "glm/glm.hpp"
#include "../Rendering.hpp"

namespace LineDrawer
{
	// draws a line between a and b
	void DrawLine(const glm::vec3& a, const glm::vec3& b);
	void SetColor(const glm::vec3& color);
	void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	void SetShader();
	void Dispose();
	void Render();
}