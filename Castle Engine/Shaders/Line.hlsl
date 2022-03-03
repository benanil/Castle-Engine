cbuffer cbPerObject : register(b0)
{
	float4x4 MVP;
	float4x4 Model;
};

struct VS_INPUT {
	float4 position	   : POSITION;
	half3 color       : COLOR;
};

struct VS_OUT {
	float4 position : SV_POSITION;
	half3 color    : COLOR;
};

VS_OUT VS(VS_INPUT i)
{
	VS_OUT o;
	i.position.w = 1;
	o.position = mul(i.position, MVP);
	o.color = i.color;
	return o;
}

float4 PS(VS_OUT i) : SV_TARGET
{
	return float4(i.color, 1);
}