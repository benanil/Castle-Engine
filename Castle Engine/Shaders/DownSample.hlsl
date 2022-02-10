#define HALF_MAX        65504.0 
#define EPSILON         1.0e-4

struct PostProcessVertex
{
	float4 pos   : SV_POSITION;
	float2 texCoord : TEXCOORD;
};

cbuffer ScreenSizeCB : register(b0)
{
	int2 screenSize; int mode; float Treshold;
};

#ifdef DEBUG
//Texture2D _texture ;
#else
Texture2D _texture  : register(t0);
Texture2D _texture1 : register(t1);

#endif
SamplerState texSampler : register(s0);
SamplerState texSampler1 : register(s1);

PostProcessVertex VS(float4 pos : POSITION, float2 uv : TEXCOORD)
{
	PostProcessVertex o;
	o.pos = pos;
	o.texCoord = uv;
	return o;
}

float Max3(float a, float b, float c)
{
	return max(max(a, b), c);
}

half4 SafeHDR(half4 c)
{
	return min(c, HALF_MAX);
}

half4 QuadraticThreshold(half4 color, half threshold, half3 curve)
{
	// Pixel brightness
	half br = Max3(color.r, color.g, color.b);

	// Under-threshold part: quadratic curve
	half rq = clamp(br - curve.x, 0.0, curve.y);
	rq = curve.z * rq * rq;

	// Combine and apply the brightness response curve.
	color *= max(rq, br - threshold) / max(br, EPSILON);

	return color;
}


// 9-tap bilinear upsampler (tent filter)
half4 UpsampleTent(float2 texCoord, float2 texelSize)
{
	int4 d = int4(1, 1, -1, 0);

	half4 s;
	s = _texture.Sample(texSampler, texCoord - (texelSize * float2(d.xy)));
	s += _texture.Sample(texSampler, texCoord - (texelSize * float2(d.wy))) * 2.0;
	s += _texture.Sample(texSampler, texCoord - (texelSize * float2(d.zy)));

	s += _texture.Sample(texSampler, texCoord + (texelSize * float2(d.zw))) * 2.0;
	s += _texture.Sample(texSampler, texCoord) * 4.0;
	s += _texture.Sample(texSampler, texCoord + (texelSize * float2(d.xw))) * 2.0;

	s += _texture.Sample(texSampler, texCoord + (texelSize * float2(d.zy)));
	s += _texture.Sample(texSampler, texCoord + (texelSize * float2(d.wy))) * 2.0;
	s += _texture.Sample(texSampler, texCoord + (texelSize * float2(d.xy)));

	return s * (1.0 / 16.0);
}

// Standard box filtering
half4 UpsampleBox(float2 texCoord, float2 texelSize)
{
	int4 d = int4(-1, -1, 1, 1);

	half4 s;
	s = _texture.Sample(texSampler, texCoord * 2 + (texelSize * float2(d.xy)));
	s += _texture.Sample(texSampler, texCoord * 2 + (texelSize * float2(d.zy)));
	s += _texture.Sample(texSampler, texCoord * 2 + (texelSize * float2(d.xw)));
	s += _texture.Sample(texSampler, texCoord * 2 + (texelSize * float2(d.zw)));

	return s * (1.0 / 4.0);
}
// Standard box filtering
half4 DownsampleBox4Tap(float2 texCoord, float2 texelSize)
{
	int4 d = int4(-1.0, -1.0, 1.0, 1.0);

	half4 s;
	s = _texture.Sample(texSampler, texCoord + (texelSize * float2(d.xy)));
	s += _texture.Sample(texSampler, texCoord + (texelSize * float2(d.zy)));
	s += _texture.Sample(texSampler, texCoord + (texelSize * float2(d.xw)));
	s += _texture.Sample(texSampler, texCoord + (texelSize * float2(d.zw)));

	return s * (1.0 / 4.0);
}

// DownsampleBox13Tap
half4 DownsampleBox13Tap(float2 texCoord, float2 texelSize)
{
	half4 A = _texture.Sample(texSampler, texCoord + (texelSize * float2(-2, -2)));
	half4 B = _texture.Sample(texSampler, texCoord + (texelSize * float2(0, -2)));
	half4 C = _texture.Sample(texSampler, texCoord + (texelSize * float2(2, -2)));
	half4 D = _texture.Sample(texSampler, texCoord + (texelSize * float2(-1, -1)));
	half4 E = _texture.Sample(texSampler, texCoord + (texelSize * float2(1, -1)));
	half4 F = _texture.Sample(texSampler, texCoord + (texelSize * float2(-2, 0)));
	half4 G = _texture.Sample(texSampler, texCoord + (texelSize * float2(0, 0)));
	half4 H = _texture.Sample(texSampler, texCoord + (texelSize * float2(2, 0)));
	half4 I = _texture.Sample(texSampler, texCoord + (texelSize * float2(-1, 1)));
	half4 J = _texture.Sample(texSampler, texCoord + (texelSize * float2(1, 1)));
	half4 K = _texture.Sample(texSampler, texCoord + (texelSize * float2(-2, 2)));
	half4 L = _texture.Sample(texSampler, texCoord + (texelSize * float2(0, 2)));
	half4 M = _texture.Sample(texSampler, texCoord + (texelSize * float2(2, 2)));

	half2 div = (1.0 / 4.0) * half2(0.5, 0.125);

	half4 o = (D + E + I + J) * div.x;
	o += (A + B + G + F) * div.y;
	o += (B + C + H + G) * div.y;
	o += (F + G + L + K) * div.y;
	o += (G + H + M + L) * div.y;
	o.a = 1;
	return o;
}
// ----------------------------------------------------------------------------------------
// Prefilter

half4 Prefilter(half4 color, float2 uv)
{
	// static const float Treshold = 0.65;
	static const float knee = 0.33f;

	// x: threshold value (linear), y: threshold - knee, z: knee * 2, w: 0.25 / knee
	float4 _Threshold = float4(Treshold, Treshold - knee, knee * 2, 0.25 / knee);
	color = QuadraticThreshold(color, _Threshold.x, _Threshold.yzw);
	return color;
}

half4 FragPrefilter13(PostProcessVertex i) : SV_Target
{
	float2 texelSize = float2(1.0, 1.0) / screenSize;
	i.texCoord.y = 1 - i.texCoord.y;
	i.texCoord *= mode;
	half4 color = DownsampleBox13Tap(i.texCoord, texelSize);
	return Prefilter(SafeHDR(color), i.texCoord);
}

half4 FragPrefilter4(PostProcessVertex i) : SV_Target
{
	float2 texelSize = float2(1.0, 1.0) / screenSize;
	i.texCoord.y = 1 - i.texCoord.y;
	i.texCoord *= mode;
	half4 color = DownsampleBox4Tap(i.texCoord, texelSize);
	return Prefilter(SafeHDR(color), i.texCoord);
}

// ----------------------------------------------------------------------------------------
// Downsample1
half4 FragDownsample13(PostProcessVertex i) : SV_Target
{
	float2 texelSize = float2(1.0, 1.0) / screenSize;
	i.texCoord.y = 1 - i.texCoord.y;
	i.texCoord *= mode;
	return DownsampleBox13Tap(i.texCoord, texelSize);
}

half4 FragDownsample4(PostProcessVertex i) : SV_Target
{
	float2 texelSize = float2(1.0, 1.0) / screenSize;
	i.texCoord.y = 1 - i.texCoord.y;
	i.texCoord *= mode;
	return DownsampleBox4Tap(i.texCoord , texelSize);
}

// ----------------------------------------------------------------------------------------
// Upsample & combine

half4 FragUpsampleTent(PostProcessVertex i) : SV_Target
{
	float2 texelSize = float2(1.0, 1.0) / screenSize;
	i.texCoord.y = 1 - i.texCoord.y;
	i.texCoord *= mode;
	half4 bloom = UpsampleTent(i.texCoord , texelSize);
	return bloom + _texture1.Sample(texSampler1, i.texCoord);
}

half4 FragUpsampleBox(PostProcessVertex i) : SV_Target
{
	float2 texelSize = float2(1.0, 1.0) / screenSize;
	i.texCoord.y = 1 - i.texCoord.y;
	i.texCoord *= mode;
	half4 bloom = UpsampleBox(i.texCoord * 0.5, texelSize);
	return bloom + _texture1.Sample(texSampler1, i.texCoord);
}