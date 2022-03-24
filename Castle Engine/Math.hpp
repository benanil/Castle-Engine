#pragma once
#include "CE_Common.hpp"

#include <windows.h>
#include <xnamath.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstdlib>
#include <array>

// NE means no except
#define NELAMBDA(x) noexcept { x;  }
#define NELAMBDAR(x) noexcept { return x;  }

#define GLM_GET_XY(vec) vec.x, vec.y
#define GLM_GET_XYZ(vec) vec.x, vec.y, vec.z 

#define GLM_SET_XY(vec, _x, _y) vec.x = _x; vec.y = _y; 
#define GLM_SET_XYZ(vec, _x, _y, _z) vec.x = _x; vec.y = _y; vec.z = _y;

#ifndef DX_INLINE 
#	ifndef _MSC_VER 
#		define DX_INLINE [[nodiscard]] static inline
#	else
#		define DX_INLINE [[nodiscard]] static __forceinline
#	endif
#endif 

#define CMathNamespace namespace CMath {
#define CMathNamespaceEnd }

constexpr float DX_RAD_TO_DEG = 57.2958f;
constexpr float DX_DEG_TO_RAD = 0.017453f;
constexpr float DX_PI = 3.1415f;
constexpr float DX_TWO_PI = 6.2831f;

typedef XMVECTORF32 xmVector;
typedef XMVECTORF32 xmQuaternion;

typedef uint8_t uint8;

CMathNamespace 

template<typename T>
DX_INLINE float Max(T a, T b) noexcept { return a > b ? a : b; }
template<typename T>
DX_INLINE float Min(T a, T b) noexcept { return a < b ? a : b; }
template<typename T>
DX_INLINE float Clamp(T x, T a, T b) noexcept { return Max(a, Min(b, x)); }
DX_INLINE float IsZero(float x) noexcept { return fabs(x) > 1e-10; }

DX_INLINE xmVector GlmToXM(const glm::vec3& vec) NELAMBDAR({ GLM_GET_XYZ(vec) })

DX_INLINE const float& XMGETX(const xmVector& vector) noexcept { return vector.f[0];  }
DX_INLINE const float& XMGETY(const xmVector& vector) noexcept { return vector.f[1];  }
DX_INLINE const float& XMGETZ(const xmVector& vector) noexcept { return vector.f[2];  }
DX_INLINE const float& XMGETW(const xmVector& vector) noexcept { return vector.f[3];  }
DX_INLINE float& XMSETX(xmVector& vector) noexcept { return vector.f[0]; }
DX_INLINE float& XMSETY(xmVector& vector) noexcept { return vector.f[1]; }
DX_INLINE float& XMSETZ(xmVector& vector) noexcept { return vector.f[2]; }
DX_INLINE float& XMSETW(xmVector& vector) noexcept { return vector.f[3]; }
DX_INLINE float XM_RadToDeg(const float& radians) noexcept { return radians* DX_RAD_TO_DEG;}
DX_INLINE float XM_DegToRad(const float& degree)  noexcept { return degree * DX_DEG_TO_RAD; }
DX_INLINE void GLM_DegToRad(const glm::vec3& degree, glm::vec3& radians) noexcept { radians = degree * DX_DEG_TO_RAD; }
DX_INLINE void GLM_RadToDeg(const glm::vec3& radians, glm::vec3& degree) noexcept { degree = radians * DX_RAD_TO_DEG; }
DX_INLINE glm::vec3 GLM_RadToDeg(const glm::vec3& radians) noexcept { return radians * DX_RAD_TO_DEG; }
DX_INLINE glm::vec3 GLM_DegToRad(const glm::vec3& radians) noexcept { return radians * DX_DEG_TO_RAD; }

DX_INLINE xmQuaternion xmEulerToQuaternion(glm::vec3 euler) noexcept // yaw (Z), pitch (Y), roll (X)
{
	// Abbreviations for the various angular functions
	euler *= 0.5f;

	float c1 = cos(euler.x);  float s1 = sin(euler.x);
	float c2 = cos(euler.y);  float s2 = sin(euler.y);
	float c3 = cos(euler.z);  float s3 = sin(euler.z);
	
	xmQuaternion q{};

	XMSETW(q) = (c1 * c2 * c3) - (s1 * s2 * s3);
	XMSETX(q) = (s1 * c2 * c3) + (c1 * s2 * s3);
	XMSETY(q) = (c1 * s2 * c3) - (s1 * c2 * s3);
	XMSETZ(q) = (c1 * c2 * s3) + (s1 * s2 * c3);

	return q;
}

DX_INLINE glm::vec3 xmQuatToEulerAngles(const xmQuaternion& q) noexcept {
	glm::vec3 eulerAngles;

	// Threshold for the singularities found at the north/south poles.
	constexpr float SINGULARITY_THRESHOLD = 0.4999995f;

	float sqw = XMGETW(q) * XMGETW(q);
	float sqx = XMGETX(q) * XMGETX(q);
	float sqy = XMGETY(q) * XMGETY(q);
	float sqz = XMGETZ(q) * XMGETZ(q);
	float unit = sqx + sqy + sqz + sqw; // if normalised is one, otherwise is correction factor
	float singularityTest = (XMGETX(q)* XMGETZ(q)) + (XMGETW(q)* XMGETY(q));

	if (singularityTest > SINGULARITY_THRESHOLD * unit)
	{
		eulerAngles.z = 2.0f * atan2(XMGETX(q), XMGETW(q));
		eulerAngles.y = DX_PI / 2;
		eulerAngles.x = 0;
	}
	else if (singularityTest < -SINGULARITY_THRESHOLD * unit)
	{
		eulerAngles.z = -2.0f * atan2(XMGETX(q), XMGETW(q));
		eulerAngles.y = -(DX_PI / 2);
		eulerAngles.x = 0;
	}
	else {
		eulerAngles.z = atan2(2 * ((XMGETW(q) * XMGETZ(q)) - (XMGETX(q) * XMGETY(q))), sqw + sqx - sqy - sqz);
		eulerAngles.y = asin(2 * singularityTest / unit);
		eulerAngles.x = atan2(2 * ((XMGETW(q) * XMGETX(q)) - (XMGETY(q) * XMGETZ(q))), sqw - sqx - sqy + sqz);
	}
	return eulerAngles;
}

DX_INLINE float xmRepeat(const float& t, const float& length) noexcept
{
	return glm::clamp(t - glm::floor(t / length) * length, 0.0f, length);
}

DX_INLINE float xmLerpAngle(const float& a, const float& b, const float& t) noexcept
{
	float delta = xmRepeat((b - a), 360);
	if (delta > 180)
		delta -= 360;
	return a + delta * glm::clamp(t, 0.0f, 1.0f);
}

DX_INLINE glm::vec3 xmExtractPosition(const XMMATRIX& matrix) noexcept
{
	XMVECTOR row3 = matrix.r[3];
	return { row3.m128_f32[1], row3.m128_f32[2] , row3.m128_f32[3] };
}

DX_INLINE glm::vec3 xmExtractScale(const XMMATRIX& matrix) noexcept
{
	XMVECTOR row0 = matrix.r[0];
	XMVECTOR row1 = matrix.r[1];
	XMVECTOR row2 = matrix.r[2];

	return { XMVector3Length(row0).m128_f32[0], XMVector3Length(row1).m128_f32[0], XMVector3Length(row2).m128_f32[0] };
}
// https://github.com/opentk/opentk/blob/master/src/OpenTK.Mathematics/Vector/Vector3.cs
DX_INLINE glm::vec3 xmVec3Transform(const glm::vec3& vec, const xmVector& q) noexcept
{
	glm::vec3 xyz = { q.f[0], q.f[1], q.f[2] };
 	glm::vec3 temp = glm::cross(xyz, vec);
	glm::vec3 temp1 = vec * q.f[3];
	temp += temp1;
	temp1 = glm::cross(xyz, temp) * 2.0f;
	return vec + temp1;
}

DX_INLINE glm::vec3 xmVec3Transform(const glm::vec3& vec, const float* q) noexcept
{
	glm::vec3 xyz = { q[0], q[1], q[2] };
	glm::vec3 temp = glm::cross(xyz, vec);
	glm::vec3 temp1 = vec * q[3];
	temp += temp1;
	temp1 = glm::cross(xyz, temp) * 2.0f;
	return vec + temp1;
}

// https://github.com/opentk/opentk/blob/master/src/OpenTK.Mathematics/Matrix/Matrix4.cs
DX_INLINE xmQuaternion xmExtractRotation(const XMMATRIX& matrix, bool rowNormalize = true) noexcept
{
	XMVECTOR row0 = matrix.r[0];
	XMVECTOR row1 = matrix.r[1];
	XMVECTOR row2 = matrix.r[2];

	if (rowNormalize) {
		row0 = XMVector3Normalize(row0);
		row1 = XMVector3Normalize(row1);
		row2 = XMVector3Normalize(row2);
	}

	// code below adapted from Blender
	xmQuaternion q;
	float trace = 0.25 * (row0.m128_f32[0] + row1.m128_f32[1] + row2.m128_f32[2] + 1.0);

	if (trace > 0)
	{
		float sq = glm::sqrt(trace);

		XMSETW(q) = sq;
		sq = 1.0 / (4.0 * sq);
		XMSETX(q) = (row1.m128_f32[2] - row2.m128_f32[1]) * sq;
		XMSETY(q) = (row2.m128_f32[0] - row0.m128_f32[2]) * sq;
		XMSETZ(q) = (row0.m128_f32[1] - row1.m128_f32[0]) * sq;
	}
	else if (row0.m128_f32[0] > row1.m128_f32[1] && row0.m128_f32[0] > row2.m128_f32[2])
	{
		float sq = 2.0 * glm::sqrt(1.0 + row0.m128_f32[0] - row1.m128_f32[1] - row2.m128_f32[2]);

		XMSETX(q) = 0.25 * sq;
		sq = 1.0 / sq;
		XMSETW(q) = (row2.m128_f32[1] - row1.m128_f32[2]) * sq;
		XMSETY(q) = (row1.m128_f32[0] + row0.m128_f32[1]) * sq;
		XMSETZ(q) = (row2.m128_f32[0] + row0.m128_f32[2]) * sq;
	}
	else if (row1.m128_f32[1] > row2.m128_f32[2])
	{
		float sq = 2.0 * glm::sqrt(1.0f + row1.m128_f32[1] - row0.m128_f32[0] - row2.m128_f32[2]);

		XMSETY(q) = 0.25 * sq;
		sq = 1.0 / sq;
		XMSETW(q) = (row2.m128_f32[0] - row0.m128_f32[2]) * sq;
		XMSETX(q) = (row1.m128_f32[0] + row0.m128_f32[1]) * sq;
		XMSETZ(q) = (row2.m128_f32[1] + row1.m128_f32[2]) * sq;
	}
	else {
		float sq = 2.0 * glm::sqrt(1.0f+ row2.m128_f32[2] - row0.m128_f32[0] - row1.m128_f32[1]);

		XMSETZ(q) = 0.25 * sq;
		sq = 1.0 / sq;
		XMSETW(q) = (row1.m128_f32[0] - row0.m128_f32[1]) * sq;
		XMSETX(q) = (row2.m128_f32[0] + row0.m128_f32[2]) * sq;
		XMSETY(q) = (row2.m128_f32[1] + row1.m128_f32[2]) * sq;
	}
	XMVector4Normalize(q);
	return q;
}

#undef NELAMBDAR
#undef NELAMBDA

DX_INLINE void GetVec3(glm::vec3*__restrict  vec, const XMVECTOR& vector) noexcept
{
	vec->x = XMVectorGetX(vector);
	vec->y = XMVectorGetY(vector);
	vec->z = XMVectorGetZ(vector);
}

DX_INLINE void GetVec4(glm::vec4*__restrict vec, const XMVECTOR& vector) noexcept
{
	_mm_store_ps(&vec->x, vector);
}

//                     ANGLE CULLING

struct AABB {
	glm::vec3 min, max;
	const glm::vec3& GetMin() const { return min; };
	const glm::vec3& GetMax() const { return max; };

	__forceinline std::array<glm::vec3, 8> GetXYZEdges() const
	{
		std::array<glm::vec3, 8> result =
		{
			glm::vec3(min.x, min.y, min.z),
			glm::vec3(max.x, max.y, max.z), //

			glm::vec3(max.x, max.y, min.z),
			glm::vec3(min.x, min.y, max.z), //

			glm::vec3(min.x, max.y, min.z),
			glm::vec3(max.x, min.y, max.z), //

			glm::vec3(max.x, min.y, min.z),
			glm::vec3(min.x, max.y, max.z)  //
		};
		return result;
	}

	__forceinline std::array<glm::vec4, 8> GetXYZEdges4(const XMMATRIX& matrix) const
	{
		std::array<glm::vec4, 8> corners =
		{
			glm::vec4(min.x, min.y, min.z, 1.0f),
			glm::vec4(max.x, max.y, max.z, 1.0f), //
			glm::vec4(max.x, max.y, min.z, 1.0f),
			glm::vec4(min.x, min.y, max.z, 1.0f), //
			glm::vec4(min.x, max.y, min.z, 1.0f),
			glm::vec4(max.x, min.y, max.z, 1.0f), //
			glm::vec4(max.x, min.y, min.z, 1.0f),
			glm::vec4(min.x, max.y, max.z, 1.0f)  //
		};

		for (auto& corner : corners) _mm_store_ps(&corner.x, XMVector3Transform(_mm_load_ps(&corner.x), matrix));
		return corners;
	}

	__forceinline std::array<glm::vec3, 8> GetXYZEdges(const XMMATRIX& matrix) const
	{
		std::array<glm::vec3, 8> corners =
		{
			glm::vec3(min.x, min.y, min.z),
			glm::vec3(max.x, max.y, max.z),
			glm::vec3(max.x, max.y, min.z),
			glm::vec3(min.x, min.y, max.z),
			glm::vec3(min.x, max.y, min.z),
			glm::vec3(max.x, min.y, max.z),
			glm::vec3(max.x, min.y, min.z),
			glm::vec3(min.x, max.y, max.z)
		};
		for (auto& corner : corners) {
			XMVECTOR V = XMVector3Transform(_mm_load_ps(&corner.x), matrix);
			GetVec3(&corner, V);
		}

		return corners;
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

	__forceinline bool IsPointInside_XZ(const glm::vec3& pos, const XMMATRIX& matrix) const
	{
		glm::vec4 _min{}; glm::vec4 _max{};
		_mm_store_ps(&_min.x, XMVector3Transform(_mm_load_ps(&min.x), matrix));
		_mm_store_ps(&_max.x, XMVector3Transform(_mm_load_ps(&max.x), matrix));
		return pos.x > _min.x && pos.x < _max.x&& pos.z > _min.z && pos.z < _max.z;
	}

	__forceinline bool IsPointInside_XZ(const glm::vec3& pos) const noexcept
	{
		return pos.x > min.x && pos.x < max.x&& pos.z > min.z && pos.z < max.z;
	}

	__forceinline bool IsPointInside(const glm::vec3& pos) const noexcept
	{
		return pos.x > min.x && pos.x < max.x&& pos.z > min.z && pos.z < max.z&& pos.y > min.y && pos.y < max.y;
	}
};

DX_INLINE float AngleBetween2D(glm::vec2 a, glm::vec2 b)
{
	return acosf((a.x * a.y) + (b.x * b.y) / 
		(sqrtf((a.x * a.x) + (b.x * b.x )) * sqrtf((b.x * b.x) + (b.y * b.y))));
}
/// <summary> this doesnt check y axis
/// for optimization before we send camForward 
///	ve must use a crossproduct for it (camright cross unitY) we dont want to calculate it for every aabb<summary/>
/// <param name="fov">radians</param>
DX_INLINE 
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

// frustum code from here: https://github.com/matt77hias/MAGE/blob/master/MAGE/Code/Engine/Math/geometry/bounding_volume.cpp
struct BoundingFrustum
{
	XMVECTOR m_planes[6];
	BoundingFrustum(const CXMMATRIX& viewProjection) noexcept {
		const auto C = XMMatrixTranspose(viewProjection);

		m_planes[0] = C.r[3] + C.r[0]; // m_left_plane  
		m_planes[1] = C.r[3] - C.r[0]; // m_right_plane 
		m_planes[2] = C.r[3] + C.r[1]; // m_bottom_plane
		m_planes[3] = C.r[3] - C.r[1]; // m_top_plane   
		m_planes[4] = C.r[2];		   // m_near_plane  
		m_planes[5] = C.r[3] - C.r[2]; // m_far_plane   

		for (std::size_t i = 0u; i < std::size(m_planes); ++i) {
			m_planes[i] = XMPlaneNormalize(m_planes[i]);
		}
	}
};

DX_INLINE const XMVECTOR MaxPointAlongNormal(const XMVECTOR& min, const XMVECTOR& max, const FXMVECTOR& n) noexcept {

	const auto control = XMVectorGreaterOrEqual(n, XMVectorZero());
	return XMVectorSelect(min, max, control);
}

DX_INLINE
bool CheckAABBCulled(const BoundingFrustum& frustum, const AABB& aabb, const XMMATRIX& matrix)
{
	XMVECTOR min = XMVector3Transform(GlmToXM(aabb.min), matrix);
	XMVECTOR max = XMVector3Transform(GlmToXM(aabb.max), matrix);

	for (std::size_t i = 0u; i < std::size(frustum.m_planes); ++i) {
		const auto p = MaxPointAlongNormal(min, max, frustum.m_planes[i]);
		const auto result = XMPlaneDotCoord(frustum.m_planes[i], p);
		if (XMVectorGetX(result) < 0.0f) {
			return false;
		}
	}
	return true;
}

DX_INLINE
bool isPointCulled(const BoundingFrustum& frustum, const glm::vec3& _point, const XMMATRIX& matrix)
{
	XMVECTOR point = XMVector3Transform(GlmToXM(_point), matrix);

	for (std::size_t i = 0u; i < std::size(frustum.m_planes); ++i) {
		const auto result = XMPlaneDotCoord(frustum.m_planes[i], point);
		if (XMVectorGetX(result) < 0.0f) {
			return false;
		}
	}
	return true;
}

struct OrthographicPlane
{
	glm::vec3 position, direction;
};

// for meshes
// <param name="planes"> center positions all4 planes </param>
static __forceinline
bool CheckAABBInFrustum(const CMath::AABB& aabb, const std::array<OrthographicPlane, 4>& planes, const XMMATRIX& matrix)
{
	for (glm::vec3& edge : aabb.GetXYZEdges(matrix))
	{
		for (int i = 0; i < 4; ++i)
		{
			glm::vec3 direction = glm::normalize(edge - planes[i].position);
			if (glm::dot(direction, planes[i].direction) < 0.1f) return false;
		}
	}
	return true;
}


//                     RAYCASTING

struct Ray {
	glm::vec3 origin, direction;
};

struct Line {
	glm::vec3 point1, point2;
	__forceinline Line (const glm::vec3& p0, const glm::vec3& p1) noexcept : point1(p0), point2(p1) {}
	glm::vec3 Direction() const noexcept { return point2 - point1; }
};

struct Line2D {
	glm::vec2 point1, point2;
	__forceinline Line2D(const glm::vec2& p0, const glm::vec2& p1) : point1(p0), point2(p1) {}
	glm::vec2 Direction() const noexcept { return point2 - point1; }
	__forceinline glm::vec2 GetXX() const noexcept { return glm::vec2(point1.x, point2.x); }
	__forceinline glm::vec2 GetYY() const noexcept { return glm::vec2(point1.y, point2.y); }
	__forceinline float MaxX() const noexcept { return Max(point1.x, point2.x); }
	__forceinline float MaxY() const noexcept { return Max(point1.y, point2.y); }
	__forceinline float MinX() const noexcept { return Min(point1.x, point2.x); }
	__forceinline float MinY() const noexcept { return Min(point1.y, point2.y); }
};

DX_INLINE
Ray CreateRay(const glm::vec3& origin, const glm::vec3& direction) noexcept { return {origin, direction}; }

DX_INLINE
Line CreateLine(const glm::vec3& p1, const glm::vec3& p2) noexcept { return { p1, p2 }; }

DX_INLINE
Line LineFromRay(const Ray& ray, float distance) noexcept { return { ray.origin, ray.origin + (ray.direction * distance) }; }

static __forceinline
Ray ScreenPointToRay(
	const glm::vec2& mousePos, const glm::vec2& screenScale, const XMMATRIX& projectionMatrix, const XMMATRIX& viewMatrix)
{
	Ray result;
	// http://www.opengl-tutorial.org/miscellaneous/clicking-on-objects/picking-with-a-physics-library/
	XMVECTOR rayStartNDC = XMVectorSet(
		(mousePos.x / screenScale.x - 0.5f) * 2.0f,
		(mousePos.y / screenScale.y - 0.5f) * 2.0f,
		-1.0f, // The near plane maps to Z=-1 in Normalized Device Coordinates
		 1.0f
	);

	XMVECTOR rayEndNDC = XMVectorSet(
		(mousePos.x / screenScale.x - 0.5f) * 2.0f,
		(mousePos.y / screenScale.y - 0.5f) * 2.0f,
		0.0f, // The near plane maps to Z=-1 in Normalized Device Coordinates
		1.0f
	);

	XMVECTOR temp;
	// The Projection matrix goes from Camera Space to NDC.
	// So inverse(ProjectionMatrix) goes from NDC to Camera Space.
	XMMATRIX inverseProjection = XMMatrixInverse(&temp, projectionMatrix);
	// The View Matrix goes from World Space to Camera Space.
	// So inverse(ViewMatrix) goes from Camera Space to World Space.
	XMMATRIX inverseView = XMMatrixInverse(&temp, viewMatrix);

	XMVECTOR startCamera = XMVector4Transform(rayStartNDC, inverseProjection); 
			 startCamera = XMVectorDivide(startCamera, XMVectorReplicate(XMVectorGetW(startCamera)));
	XMVECTOR startWorld = XMVector4Transform(startCamera, inverseView); 
			 startWorld = XMVectorDivide(startWorld, XMVectorReplicate(XMVectorGetW(startWorld)));
	XMVECTOR endCamera = XMVector4Transform(rayEndNDC, inverseProjection);
			 endCamera = XMVectorDivide(endCamera, XMVectorReplicate(XMVectorGetW(endCamera)));
	XMVECTOR endWorld = XMVector4Transform(endCamera, inverseView);
			 endWorld = XMVectorDivide(endWorld, XMVectorReplicate(XMVectorGetW(endWorld)));
	
	XMVECTOR rayDirWorld = XMVector3Normalize(endWorld - startWorld);

	_mm_store_ps(&result.origin.x, startWorld);
	result.origin.y = 1.0f - result.origin.y;
	GetVec3(&result.direction, rayDirWorld);

	return result;
}


struct Color32 {
	union {
		struct { uint8 colors[4]; };
		struct { uint8  r, g, b, a; };
		struct { uint8  x, y, z, w; };
	};

	inline Color32() noexcept : r(0), g(0), b(0), a(255) {}
	inline Color32(uint8 _r, uint8 _g, uint8 _b, uint8 _a = 255) noexcept : r(_r), g(_g), b(_b), a(_a) {}

	__forceinline constexpr uint8& operator[](uint8 index) { return colors[index]; }
	__forceinline constexpr uint8 const& operator[](uint8 index) const { return colors[index]; }
	
	__forceinline bool operator==(Color32 other) const noexcept {
		return other.a == a && other.r == r && other.g == g && other.b == b;
	}
	__forceinline bool operator!=(Color32 other) const noexcept {
		return other.a != a && other.r != r && other.g != g && other.b != b;
	}
	/// <summary> be sure that all components of the vec is not higher than 1.0f !!!  faster version of FromVec3 </summary>
	static __forceinline Color32 FromVec3LDR(const glm::vec3& vec)
	{
		return Color32(uint8(vec.x * 255.0f), uint8(vec.y * 255.0f), uint8(vec.z * 255.0f), 255);
	}
	/// <summary> be sure that all components of the vec is not higher than 1.0f !!! faster version of FromVec4 </summary>
	static __forceinline Color32 FromVec4LDR(const glm::vec4& vec)
	{
		return Color32(uint8(vec.x * 255.0f), uint8(vec.y * 255.0f), uint8(vec.z * 255.0f), uint8(vec.w * 255.0f));
	}
	static constexpr float OneDiv255 = 1.0f / 255.0f;
	
	__forceinline glm::vec3 ToVec3() const noexcept
	{
		return glm::vec3(float(r) * OneDiv255, float(g) * OneDiv255, float(b) * OneDiv255);
	}
	__forceinline glm::vec4 ToVec4() const noexcept
	{
		return glm::vec4(float(r) * OneDiv255, float(g) * OneDiv255, float(b) * OneDiv255, float(a) * OneDiv255);
	}
	static __forceinline Color32 Red()    { return Color32(255, 0, 0, 255); }
	static __forceinline Color32 Green()  { return Color32(0, 255, 0, 255); }
	static __forceinline Color32 Blue()   { return Color32(0, 0, 255, 255); }
	static __forceinline Color32 Orange() { return Color32(128, 128, 0, 255); }
};
 
struct Bool2
{
	union {
		struct { bool x, y; };
		struct { bool values[2]; };
		struct { bool a, b; };
	};
	inline Bool2() : a(false), b(false) {}
	inline Bool2(bool _a, bool _b) : a(_a), b(_b) {}
	__forceinline constexpr bool& operator[](uint8 index) { return values[index]; }
	__forceinline constexpr bool const& operator[](uint8 index) const { return values[index]; }
	__forceinline bool operator==(Bool2 other) const noexcept { return other.x == x && other.y == y; }
	__forceinline bool operator!=(Bool2 other) const noexcept { return other.x != x && other.y != y; }
};

DX_INLINE void VecMulBool(glm::vec2* vec, Bool2 _bool) { vec->x *= _bool.x; vec->y *= _bool.y; }


CMathNamespaceEnd