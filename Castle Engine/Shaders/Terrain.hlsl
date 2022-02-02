cbuffer cbPerObject : register(b0)
{
	float4x4 MVP;
	float4x4 Model;
};

cbuffer cbGlobal : register(b2)
{
	float sunAngle;
	float3 ambientColor;  // 16
	float3 sunColor;
	float textureScale;      // 32
	float3 viewPos;
	float ambientStength; // 48
};

struct VS_OUTPUT 
{
	float4 Pos : SV_POSITION;
	float3 normal : NORMAL;
	float3 vertexPos : TEXCOORD;
};

Texture2D terrainTexture : register(t0);
SamplerState textureSampler : register(s0);

Texture2D grassTexture : register(t1);
SamplerState grassSampler : register(s1);

VS_OUTPUT VS(float4 Pos : POSITION, float4 normal : NORMAL)
{
	VS_OUTPUT o;
	o.Pos = mul(Pos, MVP);
	o.normal = normal.xyz;
	o.vertexPos = Pos.xyz;
	return o;
}

float4 PS(VS_OUTPUT input) : SV_Target
{
	float4 result = float4(1, 1, 1, 1); 

	if (input.vertexPos.y < -13)
	{
		// terrain level is low use dirt texture
		result = terrainTexture.Sample(textureSampler, input.vertexPos.xz * (textureScale * 0.4f));	
		// 
		float ndl = max(dot(input.normal, float3(sin(0.2f), cos(0.2f), 0)), 0.16f);
		result.xyz *= ndl;
	}
	else
	{
		// use grass texture
		result = grassTexture.Sample(grassSampler, input.vertexPos.xz * (textureScale * 0.1f));	
		// 
		float ndl = max(dot(input.normal, float3(sin(0.2f), cos(0.2f), 0)), 0.16f);
		result.xyz *= ndl;	
	}
	
	return result;
}