
cbuffer cbPerObject
{
    float4x4 MVP;
    float4x4 Model;
};

struct VS_OUTPUT
{
    float4 Pos      : SV_POSITION;
    float2 TexCoord : TEXCOORD;
	float3 normal   : NORMAL;
};

Texture2D ObjTexture;
SamplerState ObjSamplerState;

VS_OUTPUT VS(float4 inPos : POSITION, float2 inTexCoord : TEXCOORD, float3 inNormal : NORMAL)
{
    VS_OUTPUT output;

    output.Pos = mul(inPos, MVP);
	output.normal  = mul(inNormal, Model);
	output.TexCoord = inTexCoord;

    return output;
}

float4 PS(VS_OUTPUT input) : SV_TARGET
{
	float4 _texture = ObjTexture.Sample(ObjSamplerState, input.TexCoord);
	float3 L = float3(sin(0), cos(0), 0);
	float ndl = max(dot(input.normal, L), 0.42f);
	_texture  *= ndl;
	_texture.a = 1;
	return _texture;
}