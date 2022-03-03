#pragma once
#include "../Math.hpp"
#include "../Rendering.hpp"
#include "../FreeCamera.hpp"

namespace LineDrawer2D
{
	// draws a line between a and b
	void SetColor(const Color32& color);
	const Color32& GetColor();
	void Initialize(FreeCamera* freeCamera);
	/// <summary> asign NDC cordinates of the center, [-1,-1] </summary>
	void DrawLine(const glm::vec2& a, const glm::vec2& b);
	/// <summary> asign NDC cordinates of the center, [-1,-1] </summary>
	void DrawCircale(const glm::vec2& center, float circumferance, int segments = 20);
	/// <summary> asign NDC cordinates of the center, [-1,-1] </summary>
	void DrawCube(const glm::vec2& position, const glm::vec2& scale);
	/// <summary> asign NDC cordinates of the center, [-1,-1] </summary>
	void DrawPlus(const glm::vec2& point);
	// converts to ndc space then draws
	void DrawLine(const glm::vec3& a, const glm::vec3& b);
	// converts to ndc space then draws
	void DrawThickLine(const glm::vec3& a, const glm::vec3& b);
	// converts to ndc space then draws
	void DrawCircale(const glm::vec3& center, float circumferance, int segments = 20);

	void SetShader();
	void Dispose();
	void Render();
}