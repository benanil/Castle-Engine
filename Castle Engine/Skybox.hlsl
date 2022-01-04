
cbuffer cbPerObject : register(b0)
{
	float4x4 MVP;
	float4x4 Model;
};

struct SKYMAP_VS_OUTPUT 
{
	float4 Pos : SV_POSITION;
	float3 texCoord : TEXCOORD;
};

SKYMAP_VS_OUTPUT VS(float3 inPos : POSITION)
{
	SKYMAP_VS_OUTPUT output;
	
	//Set Pos to xyww instead of xyzw, so that z will always be 1 (furthest from camera)
	output.Pos = mul(float4(inPos, 1.0f), MVP).xyww;
	
	output.texCoord = inPos;
	
	return output;
}

TextureCube SkyMap;
SamplerState SkyMapSampler; 

float4 PS(SKYMAP_VS_OUTPUT input) : SV_Target
{
	float4 result = SkyMap.Sample(SkyMapSampler, input.texCoord);
	result.a = 1;

	return result;
}
