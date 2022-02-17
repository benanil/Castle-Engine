cbuffer cbPerObject : register(b0)
{
	float4x4 MVP;
	float4x4 Model;
};

// cbuffer cbGlobal : register(b2)
// {
// 	float sunAngle;
// 	float3 ambientColor;  // 16
// 	float3 sunColor;
// 	float textureScale;   // 32
// 	float3 viewPos;
// 	float ambientStength; // 48
// };

struct VS_OUT {
	float4 position : SV_POSITION;
	half2 texCoord  : TEXCOORD;
};

struct VS_INPUT {
	float4 position	   : POSITION;
	half2 texCoord	   : TEXCOORD;
	float3 instancePos : INSTANCE_POS;
	uint instanceID	   : SV_InstanceID;
};

VS_OUT VS (VS_INPUT i)
{
	VS_OUT o;
	float4 instancePos = float4(0, 0, 0, 0);
	instancePos.xyz = i.instancePos;
	o.position = mul(i.position + instancePos, MVP);
	o.texCoord = i.texCoord;
	return o;
}

float4 PS (VS_OUT i) : SV_TARGET
{
	return float4(1,1,1,1);
}