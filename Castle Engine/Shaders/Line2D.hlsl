
struct VS_INPUT {
	float2 position	   : POSITION;
	half4 color        : COLOR;
};

struct VS_OUT {
	float4 position : SV_POSITION;
	half3 color    : COLOR;
};

VS_OUT VS(VS_INPUT i)
{
	VS_OUT o;
	o.position = float4(i.position.x, i.position.y, 0, 1);
	o.color = i.color;
	return o;
}

half4 PS(VS_OUT i) : SV_TARGET
{
	return half4(i.color, 1);
}