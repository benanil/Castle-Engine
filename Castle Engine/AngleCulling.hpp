#include <array>
#include <glm/glm.hpp>

struct AABB {
	glm::vec3 min, max;
	const glm::vec3& GetMin() const { return min; };
	const glm::vec3& GetMax() const { return max; };

	__forceinline std::array<glm::vec3, 9> GetXYZEdges() const
	{
		std::array<glm::vec3, 9> result =
		{
			glm::vec3(min.x, min.y, min.z),
			glm::vec3(max.x, max.y, max.z),

			glm::vec3(max.x, max.y, min.z),
			glm::vec3(min.x, min.y, max.z),

			glm::vec3(min.x, max.y, min.z),
			glm::vec3(max.x, min.y, max.z),

			glm::vec3(max.x, min.y, min.z),
			glm::vec3(min.x, max.y, max.z),
			glm::mix(min, max, 0.5f)
		};
		return result;
	}
	/// for terrain: returns center point aditionaly
	__forceinline std::array<glm::vec2, 5> GetXZEdges() const
	{
		std::array<glm::vec2, 5> result
		{
			glm::vec2{min.x, min.z},
			{max.x, max.z},
			{max.x, min.z},
			{min.x, max.z},
			glm::mix(result[1], result[0], 0.5f)
		};
		return result;
	}

	__forceinline bool IsPointInside_XZ(const glm::vec3& pos) const
	{
		return pos.x > min.x && pos.x < max.x&& pos.z > min.z && pos.z < max.z;
	}

	__forceinline bool IsPointInside(const glm::vec3& pos) const
	{
		return pos.x > min.x && pos.x < max.x&& pos.z > min.z && pos.z < max.z&& pos.y > min.y && pos.y < max.y;
	}
};

/// <summary> this doesnt check y axis
/// for optimization before we send camForward 
///	ve must use a crossproduct for it (right.unitY) we dont want to calculate it for every aabb<summary/>
/// <param name="fov">radians</param>
static __forceinline
bool isTerrainCulled(const AABB& aabb, const glm::vec3& camPos, const glm::vec3& camForward, float fov)
{
	if (aabb.IsPointInside_XZ(camPos)) return true; // player on terrain

	// maximum fov value is pi so we are mapping fovpercent 0-1 range for dot product
	float fovPercent = fov / glm::pi<float>();

	for (const glm::vec2& edge : aabb.GetXZEdges())
	{
		glm::vec3 edge_to_cam = glm::normalize(glm::vec3(edge.x, 0, edge.y) - glm::vec3(camPos.x, 0, camPos.z));
		if (glm::dot(edge_to_cam, camForward) > 1.0 - fovPercent) return true;
	}
	return false;
}

struct AABBCullData {
	AABB* aabb;
	const glm::vec3& camPos;
	const glm::vec3& camForward;
	const glm::vec3& camFwdNoY; // forward no y
	const float fov;
};

static __forceinline
bool CheckAABBCulled(const AABBCullData& data)
{
	if (data.aabb->IsPointInside_XZ(data.camPos)) return true; // player in box

	// maximum fov value is pi so we are mapping fovpercent 0-1 range for dot product
	float fovPercent = data.fov + 0.08f / glm::pi<float>();

	for (const glm::vec3& edge : data.aabb->GetXYZEdges())
	{
		glm::vec3 edge_to_cam = glm::normalize(glm::vec3(edge.x, 0, edge.y) - glm::vec3(data.camPos.x, 0, data.camPos.z));
		if (glm::dot(edge_to_cam, data.camForward) + glm::abs(data.camForward.y * 0.4f) > 1.0f - fovPercent) return true;
	}
	return false;
}

/// <param name="fov">radians</param>
static __forceinline
bool isPointCulled(const glm::vec3& pos, const glm::vec3& camPos, const glm::vec3& camRight, float fov)
{
	// maximum fov value is pi so we are mapping fovpercent 0-1 range for dot product
	float fovPercent = fov / glm::pi<float>();
	// for ignoring y axis ve are taking cross product of right vector (we only want xz)
	// if you want to optimize you can send precalculated forward vector 
	glm::vec3 camForward = glm::cross(camRight, glm::vec3(0, 1, 0));
	glm::vec3 point_to_cam = glm::normalize(glm::vec3(pos.x, 0, pos.y) - glm::vec3(camPos.x, 0, camPos.z));
	return glm::dot(point_to_cam, camForward) > 1.0 - fovPercent;
}
