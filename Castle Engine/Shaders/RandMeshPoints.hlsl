#define NOISE_SIMPLEX_1_DIV_289 0.00346020761245674740484429065744f

// noise from fadookie: https://gist.github.com/fadookie/25adf86ae7e2753d717c
inline float2 mod289(float2 x) { return x - floor(x * NOISE_SIMPLEX_1_DIV_289) * 289.0; }
inline float3 mod289(float3 x) { return x - floor(x * NOISE_SIMPLEX_1_DIV_289) * 289.0; }
inline float3 permute(float3 x) { return mod289(x * x * 34.0 + x); }
inline float snoise(float2 v) {
	const float4 C = float4(0.211324865405187, 0.366025403784439, -0.577350269189626, 0.024390243902439);
	float2 i = floor(v + dot(v, C.yy));
	float2 x0 = v - i + dot(i, C.xx);
	int2 i1 = (x0.x > x0.y) ? float2(1.0, 0.0) : float2(0.0, 1.0);
	float4 x12 = x0.xyxy + C.xxzz;
	x12.xy -= i1; i = mod289(i); // Avoid truncation effects in permutation
	float3 p = permute(permute(i.y + float3(0.0, i1.y, 1.0)) + i.x + float3(0.0, i1.x, 1.0));
	float3 m = max(0.5 - float3(dot(x0, x0), dot(x12.xy, x12.xy), dot(x12.zw, x12.zw)), 0.0);
	m = m * m; m = m * m;
	float3 x = 2.0 * frac(p * C.www) - 1.0;
	float3 h = abs(x) - 0.5;
	float3 ox = floor(x + 0.5);
	float3 a0 = x - ox;
	m *= 1.79284291400159 - 0.85373472095314 * (a0 * a0 + h * h);
	float3 g;
	g.x = a0.x * x0.x + h.x * x0.y;
	g.yz = a0.yz * x12.xz + h.yz * x12.yw;
	return 130.0 * dot(m, g);
}

struct Vertex {
	float3 pos; 
	float pad;
	float3 normal; 
	float pad1;
};

StructuredBuffer<Vertex> vertices;
StructuredBuffer<uint> indices;
RWStructuredBuffer<float4x4> results;
RWStructuredBuffer<int> culledTriangles;

uint hash(uint x) {
	x += (x << 10u);
	x ^= (x >> 6u);
	x += (x << 3u);
	x ^= (x >> 11u);
	x += (x << 15u);
	return x;
}

uint2 hash(uint2 x) {
	return uint2(hash(x.x), hash(x.y));
}

void Mantissa(inout uint x) {
	const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
	const uint ieeeOne = 0x3F800000u; // 1.0 in IEEE binary32
	x &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
	x |= ieeeOne;                          // Add fractional part to 1.0
}
// https://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl
// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct(uint2 m) {

	Mantissa(m.x); Mantissa(m.y);
	float2 f2 = asfloat(m) - float2(1.0, 1.0); // Range [0:1]
	return f2.x + f2.y * 0.5 + 0.1f;                    // Range [0:1]
}
// returns number between 0-1
float rand(uint2 v) { return floatConstruct(hash(v)); }

float4x4 Scale(in float3 scale)  {
	return float4x4(scale.x,	   0,       0, 0,
					0      , scale.y,       0, 0,
					0      ,       0, scale.z, 0,
					0      ,       0,       0, 1);
}


float4x4 CreatePosMatrix(float3 pos) {
	return float4x4(1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		pos.x , pos.y, pos.z, 1);
}

static const float4x4 Identity = {
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1
};

inline float4x4 Rotate(float angle, float3 axis) {
	float c, s;
	sincos(angle, s, c);

	float t = 1 - c;
	float x = axis.x;
	float y = axis.y;
	float z = axis.z;

	return float4x4(
		t * x * x + c, t * x * y - s * z, t * x * z + s * y, 0,
		t * x * y + s * z, t * y * y + c, t * y * z - s * x, 0,
		t * x * z - s * y, t * y * z + s * x, t * z * z + c, 0,
		0,0,0,1);
}

#define GRASS_PER_TRIANGLE 6

inline float3 CalculateSurfaceNormal(in float3 p1, in float3 p2, in float3 p3)
{
	float3 u = p2 - p1; 
	float3 v = p3 - p1;
	float3 result = float3(0, 0, 0);
	result.x = (u.y * v.z) - (u.z * v.y);
	result.y = (u.z * v.x) - (u.x * v.z);
	result.z = (u.x * v.y) - (u.y * v.x);
	return normalize(result) ;
}

[numthreads(64, 1, 1)]
void CS(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
	const int indexCoord = dispatchThreadID.x * 3;

	const float3 v0 = vertices[indices[indexCoord + 0]].pos;
	const float3 v1 = vertices[indices[indexCoord + 1]].pos;
	const float3 v2 = vertices[indices[indexCoord + 2]].pos;

	const uint2 randOffset = groupThreadID.xy * dispatchThreadID.xy;

	const float3 normalCenter = CalculateSurfaceNormal(vertices[indices[indexCoord + 0]].pos,
												       vertices[indices[indexCoord + 1]].pos,
												       vertices[indices[indexCoord + 2]].pos);
	culledTriangles[dispatchThreadID.x] = 0;
	// we dont want to draw grass everywhere so we are reducing some grass with simpliex noise here
	if (snoise(v1.xz / 10) > 0.5 || v1.y < 0) return;

	culledTriangles[dispatchThreadID.x] = 1;

	for (int i = 0; i < GRASS_PER_TRIANGLE; ++i)
	{
		float r1 = rand(randOffset.xy + 33.55 * i);
		float r2 = rand(randOffset.yx + 66.66 * i);
		float r3 = rand(randOffset.yx + 83.66 * i);
		
		if (r1 + r2 > 1) {
			r1 = 1.0f - r1;
			r2 = 1.0f - r2;
		}
		float a = 1.0f - r1 - r2;
		
		float3 lerp2 = a * v0 + r1 * v1 + r2 * v2;

		float4x4 rotation = Rotate(rand(randOffset.xy * 133 + (i + i * 0.22)), normalCenter);
		float4x4 scale = Scale(float3(1.15, 1.15, 1.15) + (float3(r1,r2,r3) * 1.7));

		results[dispatchThreadID.x * GRASS_PER_TRIANGLE + i] =
			mul(mul(rotation, scale), CreatePosMatrix(lerp2));
	}
}
