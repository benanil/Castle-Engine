cbuffer cbPerObject : register(b0)
{
	float4x4 MVP;
	float4x4 LightSpaceMatrix;
};

cbuffer cbGlobal : register(b1)
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
	float4 LightSpaceFrag : TEXCOORD2;
};
 
struct VS_Shadow_Output {
	float4 Pos : SV_POSITION;
};

Texture2D terrainTexture : register(t0);
SamplerState textureSampler : register(s0);

Texture2D grassTexture : register(t1);
SamplerState grassSampler : register(s1);

Texture2D ShadowTexture : register(t2);
SamplerState ShadowSampler : register(s2);

VS_OUTPUT VS(float4 Pos : POSITION, float4 normal : NORMAL)
{
	VS_OUTPUT o; normal.w = 0;
	o.Pos = mul(Pos, MVP);
	o.normal = normal.xyz;//mul(normal, Model);
	o.vertexPos = Pos.xyz;
	o.LightSpaceFrag = mul(Pos, LightSpaceMatrix);
	return o;
}

VS_Shadow_Output VSShadow(float4 Pos : POSITION, float4 normal : NORMAL)
{
	VS_Shadow_Output o;
	o.Pos = mul(Pos, MVP);
	return o;
}

void PSShadow(VS_Shadow_Output i) { }

SamplerState ShadowTextureSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = BORDER;
	AddressV = BORDER;
	BorderColor = float4(0, 0, 0, 0);
};

float ShadowCalculation(float4 lpos, float ndl)
{
	static const int ShadowMapSize = 2048 << 2;
	// transform to [0,1] range
	// if we use perspective projection in future we must divide lpos.xyz by lpos.w
	lpos.x = lpos.x * 0.5 + 0.5;
	lpos.y = lpos.y * -0.5 + 0.5;

	// get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
	float closestDepth = ShadowTexture.Sample(ShadowSampler, lpos.xy).r;

	// get depth of current fragment from light's perspective
	float currentDepth = lpos.z;
	float shadow = 0.0;
	// float shadow = currentDepth - bias > closestDepth  ? .44: 1.0;  
	float2 texelSize = float2(1.0, 1.0) / float2(float(ShadowMapSize), float(ShadowMapSize));
	static const float bias = 0.0004f;
	const float realBias = max(bias * (1.0f - ndl), bias);

	[unroll(9)]
	for (int x = -1; x <= 1; ++x)
	{
		for (int y = -1; y <= 1; ++y)
		{
			float pcfDepth = ShadowTexture.Sample(ShadowTextureSampler, lpos.xy + float2(x, y) * texelSize).r;
			shadow += currentDepth - realBias > pcfDepth ? .40 : 1;
		}
	}

	if (lpos.x < 0 || lpos.x > 1 || lpos.y < 0 || lpos.y > 1) return 1.0f;
	
	return shadow / 9.0;
}

float4 PS(VS_OUTPUT input) : SV_Target
{
	float4 result = float4(1, 1, 1, 1); 

	if (input.vertexPos.y < -13) { // terrain level is low use dirt texture
	result = terrainTexture.Sample(textureSampler, input.vertexPos.xz * (textureScale * 0.4f));	
	}
	else { // use grass texture
	result = grassTexture.Sample(grassSampler, input.vertexPos.xz * (textureScale * 0.1f));	
	}
	float ndl = max(dot(input.normal, float3(sin(0.2f), cos(0.2f), 0)), 0.16f);
	result.xyz *= ndl;
	result.xyz *= ShadowCalculation(input.LightSpaceFrag, ndl);

	return  result;
}