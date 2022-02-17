

struct VS_OUT {
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD;
};

VS_OUT VS(uint id : SV_VERTEXID)
{
	VS_OUT o;

	o.position.x = (float)(id / 2) * 4 - 1;
	o.position.y = (float)(id % 2) * 4 - 1;
	o.position.z = 0.0; o.position.w = 1.0;
	
	o.texCoord.x = (float)(id / 2) * 2.0;
	o.texCoord.y = 1.0 - (float)(id % 2) * 2.0;
	
	return o;
}

Texture2D _texture;
SamplerState _sampler;

float4 PS(VS_OUT i) : SV_TARGET
{
	i.texCoord.y = 1.0 - i.texCoord.y;
	return _texture.Sample(_sampler, i.texCoord);
}