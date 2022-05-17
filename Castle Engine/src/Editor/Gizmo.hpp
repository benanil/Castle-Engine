#pragma once
#include "../Math.hpp"
#include "../FreeCamera.hpp"

namespace Gizmo
{
	enum class Mode {
		None, Position, Rotation, Scale
	};
	void Initialize(FreeCamera* freeCamera);
	/// <summary> do not call it other than main loop </summary>
	void Begin(const glm::vec2& windowScale, const glm::vec2& mousePosition, const XMMATRIX& projection, const XMMATRIX& viewMatrix);
	/// <summary> set manipulation mode: none, position, scale, rotation </summary>
	void SetMode(Mode mode);
	void Manipulate(XMMATRIX* vector, glm::vec3* position, xmQuaternion* quaternion, glm::vec3* scale);
}