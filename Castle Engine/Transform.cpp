
#include "Transform.hpp"
#include "Math.hpp"
#ifndef NEDITOR
#	include "Editor/Editor.hpp"
#endif

using namespace CMath;

namespace ECS
{
#ifndef NEDITOR
	void Transform::OnEditor()
	{
		ImGui::TextColored(HEADER_COLOR, "Transform");
		if (ImGui::DragFloat3("Position"    , glm::value_ptr(position)   , 0.1f )) UpdateTransform();
		if (ImGui::DragFloat3("Euler Angles", glm::value_ptr(eulerDegree), 0.1f )) SetEulerDegree(eulerDegree, true); 
		if (ImGui::DragFloat3("Scale"       , glm::value_ptr(scale)      , 0.01f)) UpdateTransform();
	}
#endif
	void Transform::UpdateTransform()
	{
		XMMATRIX rotation    = XMMatrixRotationRollPitchYaw(GLM_GET_XYZ(euler));
		XMMATRIX translation = XMMatrixTranslation(GLM_GET_XYZ(position));
		XMMATRIX scaleMat    = XMMatrixScaling(GLM_GET_XYZ(scale));
		
		XMQuaternionRotationRollPitchYaw(GLM_GET_XYZ(euler));

		matrix = XMMatrixIdentity() * rotation * scaleMat * translation;
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
		quaternion = xmEulerToQuaternion(euler);
		if (notify) UpdateTransform();
	}
	
	void Transform::SetEulerRadians(const glm::vec3& _euler, bool notify) noexcept
	{
		GLM_RadToDeg(_euler, eulerDegree);
		euler = _euler;
		quaternion = xmEulerToQuaternion(euler);
		if (notify) UpdateTransform();
	}
	
	void Transform::SetMatrix(const XMMATRIX& _matrix, bool notify) noexcept
	{
		matrix = _matrix;
		SetQuaternion(xmExtractRotation(_matrix), false);
		scale = xmExtractScale(_matrix);
		position = xmExtractPosition(_matrix);
		if (notify) UpdateTransform();
	}

	void Transform::SetQuaternion(const xmQuaternion& _quat, bool notify) noexcept
	{
		quaternion = _quat;
		euler = xmQuatToEulerAngles(_quat);
		eulerDegree = GLM_RadToDeg(euler);
		if (notify) UpdateTransform();
	}

	const glm::vec3 Transform::GetRight()  const noexcept
	{
		return xmVec3Transform({ 1,0,0 }, XMQuaternionConjugate(quaternion).m128_f32);
	}
	
	const glm::vec3 Transform::GetUP() const noexcept
	{
		return xmVec3Transform({ 0,1,0 }, XMQuaternionConjugate(quaternion).m128_f32);
	}
	
	const glm::vec3 Transform::GetForward() const noexcept
	{
		return xmVec3Transform({ 0,0,1 }, XMQuaternionConjugate(quaternion).m128_f32);
	}

}