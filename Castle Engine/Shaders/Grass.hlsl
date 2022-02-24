

cbuffer cbPerObject : register(b0)
{
	float4x4 ViewProjection;
	float _Time; 
	float3 color;
	float3 windDir; 
	float _windSpeed;
};

struct VS_OUT {
	float4 position : SV_POSITION;
	half2 texCoord  : TEXCOORD;
	float3 color    : COLOR;
	float3 normal   : NORMAL;
};

Texture2D _texture;
SamplerState _sampler;

StructuredBuffer<float4x4> matrices;

VS_OUT VS(float4 position : POSITION,
	half2 texCoord : TEXCOORD,
	uint instanceID : SV_InstanceID
)
{
	VS_OUT o;

	const float4 pos = mul(position, matrices[instanceID]);
	const float4x4 MVP = mul(matrices[instanceID], ViewProjection);

	// wind animation
	position += float4(windDir * _windSpeed * (1 - texCoord.y)* sin(_Time * 0.002 + pos.x + (pos.y * 2)), 0);

	o.position = mul(position, MVP);
	o.normal = mul(float4(0, 0, 1, 0), matrices[instanceID]).xyz;
	o.texCoord = texCoord;
	o.color = color;
	return o;
}

float4 PS(VS_OUT i, bool frontFace : SV_IsFrontFace) : SV_TARGET
{
	float3 normal = i.normal * (frontFace * 2 - 1);
	float4 albedo = _texture.Sample(_sampler, i.texCoord) * float4(i.color, 1);
	albedo.xyz *= max(1 - i.texCoord.y, 0.4);
	//  max(dot(normal, float3(sin(0.2), cos(0.2), 0)), 0.5f) * 
	clip(albedo.a - 0.15f);

	return albedo;
}