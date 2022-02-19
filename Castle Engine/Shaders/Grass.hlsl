cbuffer cbPerObject : register(b0)
{
	float4x4 MVP;
	float4x4 Model;
};

struct VS_OUT {
	float4 position : SV_POSITION;
	half2 texCoord  : TEXCOORD;
	bool water       : WATER;
};

struct VS_INPUT {
	float4 position	   : POSITION;
	half2 texCoord	   : TEXCOORD;
};

Texture2D _texture;
SamplerState _sampler;

VS_OUT VS(VS_INPUT i)
{
	VS_OUT o;
	o.position = mul(i.position, MVP);
	o.texCoord = i.texCoord;
	o.water = i.position.y < 0;
	return o;
}

float4 PS(VS_OUT i) : SV_TARGET
{
	if (i.water) discard;
	float4 color = _texture.Sample(_sampler, i.texCoord);
	clip(color.a - 0.20f);
	return color;
}