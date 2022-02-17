
struct Vertex
{
	float3 pos; float pad;
	float3 normal; float pad1;
};

StructuredBuffer<Vertex> vertices;
StructuredBuffer<int> indices;
RWStructuredBuffer<float3> results;

uint hash( uint x ) {
	x += ( x << 10u );
	x ^= ( x >>  6u );
	x += ( x <<  3u );
	x ^= ( x >> 11u );
	x += ( x << 15u );
	return x;
}

uint2 hash( uint2 x ) {
	return uint2(hash(x.x), hash(x.y));
}

void Mantissa(inout uint x) {
	const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
	const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32
	x &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
	x |= ieeeOne;                          // Add fractional part to 1.0
}
// https://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl
// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct( uint2 m ) {
	
	Mantissa(m.x); Mantissa(m.y);
	float2 f2 = asfloat( m ) - float2(1.0, 1.0); // Range [0:1]
	return f2.x + f2.y * 0.5;                    // Range [0:1]
}

float rand(float2 v) { return floatConstruct(hash(asuint(v))); }

//[numthreads(8, 8, 1)]
[numthreads(64, 1, 1)]
void CS(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
	float3 v0 = vertices[indices[dispatchThreadID.x * 3 + 0]].pos;
	float3 v1 = vertices[indices[dispatchThreadID.x * 3 + 1]].pos;
	float3 v2 = vertices[indices[dispatchThreadID.x * 3 + 2]].pos;
	
	float3 centerPoint = (v0 + v1 + v2) * 0.33;
	float2 randOffset = groupThreadID.xy * dispatchThreadID.xx;
	
	float3 lerp0 = lerp(centerPoint, v0, rand(randOffset.xy * 33.33f));
	float3 lerp1 = lerp(centerPoint, v1, rand(randOffset.yx * 66.66f));
	float3 lerp2 = lerp(centerPoint, v2, rand(randOffset.xy * 88.88f));
	
	results[dispatchThreadID.x] = lerp2;
}