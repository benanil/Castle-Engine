#include "Transform.hpp"
#include "Editor/Editor.hpp"

namespace ECS
{
	
	void Transform::OnEditor()
	{
		ImGui::TextColored(HEADER_COLOR, "Transform");
		if (ImGui::DragFloat3("Position"    , glm::value_ptr(position)   , 0.1f )) UpdateTransform();
		if (ImGui::DragFloat3("Euler Angles", glm::value_ptr(eulerDegree), 0.1f )) SetEulerDegree(eulerDegree, true); 
		if (ImGui::DragFloat3("Scale"       , glm::value_ptr(scale)      , 0.01f)) UpdateTransform();
	}

	void Transform::UpdateTransform()
	{
		xmMatrix rotation    = XMMatrixRotationRollPitchYaw(GLM_GET_XYZ(euler));
		xmMatrix translation = XMMatrixTranslation(GLM_GET_XYZ(position));
		xmMatrix scaleMat    = XMMatrixScaling(GLM_GET_XYZ(scale));
		
		matrix = XMMatrixIdentity() * translation * rotation * scaleMat;
		OnTransformChanged();
	}

	void Transform::SetPosition(const glm::vec3& _position, bool notify) noexcept
	{
		position = _position;
		if (notify) UpdateTransform();
	}
	
	void Transform::SetScale(const glm::vec3& _scale, bool notify) noexcept
	{
		scale = _scale;
		if (notify) UpdateTransform();
	}
	
	void Transform::SetEulerDegree(const glm::vec3& _euler, bool notify) noexcept
	{
		eulerDegree = _euler;
		GLM_DegToRad(_euler, euler);
		if (notify) UpdateTransform();
	}
	
	void Transform::SetEulerRadians(const glm::vec3& _euler, bool notify) noexcept
	{
		GLM_RadToDeg(_euler, eulerDegree);
		euler = _euler;
		if (notify) UpdateTransform();
	}
	
	void Transform::SetMatrix(const xmMatrix& _matrix, bool notify) noexcept
	{
		// todo finish
		matrix = _matrix;
	}
}