#pragma once
#include "Helper.hpp"

// NE means no except
#define NELAMBDA(x) noexcept { x;  }
#define NELAMBDAR(x) noexcept { return x;  }

#define XM_GET_XYZ(x) XMGETX(x), XMGETY(x), XMGETZ(x)

#define DX_RAD_TO_DEG 57.2958f
#define DX_DEG_TO_RAD 0.017453f
#define DX_PI 3.1415f
#define DX_TWO_PI 6.2831f

#define GLM_GET_XY(vec) vec.x, vec.y
#define GLM_GET_XYZ(vec) vec.x, vec.y, vec.z 

#define GLM_SET_XY(vec, _x, _y) vec.x = _x; vec.y = _y; 
#define GLM_SET_XYZ(vec, _x, _y, _z) vec.x = _x; vec.y = _y; vec.z = _y;

#define DX_INLINE [[nodiscard]] __forceinline 

DX_INLINE bool Approximately(float a, float b)
{
	return std::abs(a - b) < 0.1f;
}

// XMMATH
typedef XMVECTORF32 xmVector;
typedef XMMATRIX xmMatrix;
typedef XMVECTORF32 xmQuaternion;

DX_INLINE xmVector GlmToXM(const glm::vec3& vec) NELAMBDAR({ GLM_GET_XYZ(vec) })

DX_INLINE float* XMPTR(xmVector& vector)
{
	return reinterpret_cast<float*>(&vector);
}

DX_INLINE float* XMPTR(xmMatrix& vector)
{
	return &vector._11;
}

DX_INLINE const float& XMGETX(const xmVector& vector) noexcept { return vector.f[0];  }
DX_INLINE const float& XMGETY(const xmVector& vector) noexcept { return vector.f[1];  }
DX_INLINE const float& XMGETZ(const xmVector& vector) noexcept { return vector.f[2];  }
DX_INLINE const float& XMGETW(const xmVector& vector) noexcept { return vector.f[3];  }

DX_INLINE float& XMSETX(xmVector& vector) noexcept { return vector.f[0]; }
DX_INLINE float& XMSETY(xmVector& vector) noexcept { return vector.f[1]; }
DX_INLINE float& XMSETZ(xmVector& vector) noexcept { return vector.f[2]; }
DX_INLINE float& XMSETW(xmVector& vector) noexcept { return vector.f[3]; }


DX_INLINE float XM_RadToDeg(const float& radians) NELAMBDAR(radians* DX_RAD_TO_DEG)
DX_INLINE float XM_DegToRad(const float& degree)  NELAMBDAR(degree* DX_DEG_TO_RAD)

// GLM EULER

DX_INLINE void GLM_DegToRad(const glm::vec3& radians, glm::vec3& degree) noexcept
{
	degree.x = radians.x * DX_DEG_TO_RAD;
	degree.y = radians.y * DX_DEG_TO_RAD;
	degree.z = radians.z * DX_DEG_TO_RAD;
}

DX_INLINE void GLM_RadToDeg(const glm::vec3& radians, glm::vec3& degree) noexcept
{
	degree.x = radians.x * DX_RAD_TO_DEG;
	degree.y = radians.y * DX_RAD_TO_DEG;
	degree.z = radians.z * DX_RAD_TO_DEG;
}

DX_INLINE glm::vec3 GLM_RadToDeg(const glm::vec3& radians) noexcept
{
	glm::vec3 degree{};
	degree.x = radians.x * DX_RAD_TO_DEG;
	degree.y = radians.y * DX_RAD_TO_DEG;
	degree.z = radians.z * DX_RAD_TO_DEG;
	return std::move(degree);
}

DX_INLINE glm::vec3 GLM_DegToRad(const glm::vec3& radians) noexcept
{
	glm::vec3 degree{};
	degree.x = radians.x * DX_DEG_TO_RAD;
	degree.y = radians.y * DX_DEG_TO_RAD;
	degree.z = radians.z * DX_DEG_TO_RAD;
	return std::move(degree);
}

DX_INLINE xmQuaternion xmEulerToQuaternion(glm::vec3&euler) noexcept // yaw (Z), pitch (Y), roll (X)
{
	// Abbreviations for the various angular functions
	euler *= 0.5f;

	float c1 = cos(euler.x);
	float c2 = cos(euler.y);
	float c3 = cos(euler.z);
	float s1 = sin(euler.x);
	float s2 = sin(euler.y);
	float s3 = sin(euler.z);

	xmQuaternion q{};

	XMSETW(q) = (c1 * c2 * c3) - (s1 * s2 * s3);
	XMSETX(q) = (s1 * c2 * c3) + (c1 * s2 * s3);
	XMSETY(q) = (c1 * s2 * c3) - (s1 * c2 * s3);
	XMSETZ(q) = (c1 * c2 * s3) + (s1 * s2 * c3);

	return std::move(q);
}

DX_INLINE glm::vec3 xmQuatToEulerAngles(xmQuaternion q) noexcept {
	glm::vec3 eulerAngles;

	// Threshold for the singularities found at the north/south poles.
	static const float SINGULARITY_THRESHOLD = 0.4999995f;

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
	else
	{
		eulerAngles.z = atan2(2 * ((XMGETW(q) * XMGETZ(q)) - (XMGETX(q) * XMGETY(q))), sqw + sqx - sqy - sqz);
		eulerAngles.y = asin(2 * singularityTest / unit);
		eulerAngles.x = atan2(2 * ((XMGETW(q) * XMGETX(q)) - (XMGETY(q) * XMGETZ(q))), sqw - sqx - sqy + sqz);
	}
	return std::move(eulerAngles);
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

DX_INLINE glm::vec3 xmExtractPosition(const xmMatrix& matrix) noexcept
{
	XMVECTOR row3 = matrix.r[3];
	return { row3.m128_f32[1], row3.m128_f32[2] , row3.m128_f32[3] };
}

DX_INLINE glm::vec3 xmExtractScale(const xmMatrix& matrix) noexcept
{
	XMVECTOR row0 = matrix.r[0];
	XMVECTOR row1 = matrix.r[1];
	XMVECTOR row2 = matrix.r[2];

	return { XMVector3Length(row0).m128_f32[0], XMVector3Length(row1).m128_f32[0], XMVector3Length(row2).m128_f32[0] };
}
// https://github.com/opentk/opentk/blob/master/src/OpenTK.Mathematics/Vector/Vector3.cs
DX_INLINE glm::vec3 xmVec3Transform(const glm::vec3& vec, const xmVector& q) noexcept
{
	glm::vec3 result{};
	glm::vec3 xyz = { q.f[0], q.f[1], q.f[2] };
 	glm::vec3 temp = glm::cross(xyz, vec);
	glm::vec3 temp1 = vec * q.f[3];
	temp += temp1;
	temp1 = glm::cross(xyz, temp) * 2.0f;
	return vec + temp1;
}

DX_INLINE glm::vec3 xmVec3Transform(const glm::vec3& vec, const float* q)noexcept
{
	glm::vec3 result{};
	glm::vec3 xyz = { q[0], q[1], q[2] };
	glm::vec3 temp = glm::cross(xyz, vec);
	glm::vec3 temp1 = vec * q[3];
	temp += temp1;
	temp1 = glm::cross(xyz, temp) * 2.0f;
	return vec + temp1;
}

// https://github.com/opentk/opentk/blob/master/src/OpenTK.Mathematics/Matrix/Matrix4.cs
DX_INLINE xmQuaternion xmExtractRotation(const xmMatrix& matrix, bool rowNormalize = true) noexcept
{
	XMVECTOR row0 = matrix.r[0];
	XMVECTOR row1 = matrix.r[1];
	XMVECTOR row2 = matrix.r[2];

	if (rowNormalize)
	{
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
	else
	{
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