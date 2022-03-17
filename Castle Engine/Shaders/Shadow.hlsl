
struct VS_INPUT {
	float4 inPos : POSITION;
	float2 inTexCoord : TEXCOORD; 
	float4 inNormal : NORMAL;
	float4 inTangent : TANGENT;
};

struct VS_OUTPUT {
	float4 position : SV_POSITION;
};

cbuffer cbPerObject : register(b0) {
	float4x4 MVP;
};

VS_OUTPUT VS(VS_INPUT i)
{
	VS_OUTPUT output;
	// i.inPos.w = 1;
	output.position = mul(i.inPos, MVP);
	return output;
}

void PS(VS_OUTPUT i) { }