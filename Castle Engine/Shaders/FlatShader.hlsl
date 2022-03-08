

struct PostProcessVertex
{
	float4 pos   : SV_POSITION;
	centroid noperspective float2 texCoord : TEXCOORD;
};

Texture2D _texture     : register(t0);
SamplerState textureSampler  : register(s0);

PostProcessVertex VS(uint id : SV_VERTEXID)
{
	PostProcessVertex o;
	o.pos.x = (float)(id / 2) * 4 - 1;
	o.pos.y = (float)(id % 2) * 4 - 1;
	o.pos.z = 0.0; o.pos.w = 1.0;
	o.texCoord.x = (float)(id / 2) * 2.0;
	o.texCoord.y = (float)(id % 2) * 2.0;
	return o;
}

float4 PS(PostProcessVertex i) : SV_Target
{
	float4 result = _texture.Sample(textureSampler, i.texCoord);
	result.a = 1;
	return result;
}