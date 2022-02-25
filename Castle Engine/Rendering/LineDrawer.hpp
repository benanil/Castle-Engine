#pragma once
#include "../Math.hpp"
#include "../Rendering.hpp"

namespace LineDrawer
{
	// draws a line between a and b
	void DrawLine(const glm::vec3& a, const glm::vec3& b);
	void SetColor(const glm::vec3& color);
	void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	void DrawAABB(const AABB& aabb, bool active);
	void DrawCircale(const glm::vec3& center, float circumferance, int segments = 20);
	void SetMatrix(const XMMATRIX& matrix);
	void DrawCube(const glm::vec3& position, const glm::vec3& scale);
	void DrawPlus(const glm::vec3& point);
	void SetShader();
	void Dispose();
	void Render();
}