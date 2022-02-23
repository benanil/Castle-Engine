#pragma once
#include "Rendering.hpp"
#define GLM_SWIZZLE
#define GLM_FORCE_PURE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE 
#include <glm/glm.hpp>
#include "Main/Event.hpp"
#include "Math.hpp"

namespace ECS
{
	class Transform
	{
	public:
		xmMatrix matrix;
		xmQuaternion quaternion;

		glm::vec3 position;
		glm::vec3 euler;
		glm::vec3 eulerDegree;
		glm::vec3 scale;
		Event OnTransformChanged;
	public:
		Transform()
		:	matrix(XMMatrixIdentity()),
			scale ({ 1.0f,1.0f,1.0f }) {};
		
		Transform(const glm::vec3& pos, const glm::vec3& _euler, const glm::vec3& _scale = { 1, 1, 1 })
		:	position(pos), eulerDegree(_euler), euler(GLM_DegToRad(_euler)), scale(_scale)
		{ 
			UpdateTransform(); 
		}

		void SetPosition	(const glm::vec3& _position, bool notify = true) noexcept;
		void SetScale		(const glm::vec3& _scale   , bool notify = true) noexcept;
		void SetEulerDegree (const glm::vec3& _euler   , bool notify = true) noexcept;
		void SetEulerRadians(const glm::vec3& _euler   , bool notify = true) noexcept;
		void SetMatrix		(const xmMatrix& _matrix   , bool notify = true) noexcept;
		void SetQuaternion  (const xmQuaternion& quaternion, bool notify = true) noexcept;

		const glm::vec3 GetRight()   const noexcept;
		const glm::vec3 GetUP()      const noexcept;
		const glm::vec3 GetForward() const noexcept;

		void OnEditor();
		void UpdateTransform();

		const glm::vec3& GetPosition()      const noexcept LAMBDAR(position)
		const glm::vec3& GetScale()         const noexcept LAMBDAR(scale)
		const glm::vec3& GetEulerDegree()   const noexcept LAMBDAR(GLM_RadToDeg(euler))
		const glm::vec3& GetEulerRadians()  const noexcept LAMBDAR(euler)
		const xmMatrix& GetMatrix()         const noexcept LAMBDAR(matrix)
		const xmQuaternion& GetQuaternion() const noexcept LAMBDAR(quaternion)
	};
}
