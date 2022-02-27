#pragma once
#include "../Math.hpp"

namespace Gizmo
{
	void Begin(const glm::vec2& windowScale, const glm::vec2& mousePosition, const XMMATRIX& projection, const XMMATRIX& viewMatrix);
	void Manipulate(XMMATRIX& vector);
}