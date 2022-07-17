

struct PostProcessVertex
{
	float4 pos   : SV_POSITION;
	noperspective float2 texCoord : TEXCOORD;
};

Texture2D _texture     : register(t0);
Texture2D bloomTexture : register(t1);
Texture2D ssaoTexture  : register(t2);
Texture2D depthTexture : register(t3);

SamplerState textureSampler  : register(s0);
SamplerState textureSampler1 : register(s1);
SamplerState textureSampler2 : register(s2);
SamplerState depthSampler    : register(s3);

PostProcessVertex VS(uint id : SV_VERTEXID)
{
	PostProcessVertex o;
	
	o.pos.x = (float)(id / 2) * 4 - 1;
	o.pos.y = (float)(id % 2) * 4 - 1;
	o.pos.z = 0.0; o.pos.w = 1.0;
	
	o.texCoord.x = (float)(id / 2) * 2.0;
	//#ifdef RELEASE
	//o.texCoord.y = 1.0f - ((float)(id % 2) * 2.0);
	//#else
	o.texCoord.y = (float)(id % 2) * 2.0;
	//#endif

	return o;
}

// AMD Cauldron code
// 
// Copyright(c) 2020 Advanced Micro Devices, Inc.All rights reserved.
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "SoftWWAware"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.


//--------------------------------------------------------------------------------------
// AMD Tonemapper
//--------------------------------------------------------------------------------------
// General tonemapping operator, build 'b' term.
float ColToneB(float hdrMax, float contrast, float shoulder, float midIn, float midOut)
{
	return
		-((-pow(midIn, contrast) + (midOut * (pow(hdrMax, contrast * shoulder) * pow(midIn, contrast) -
		pow(hdrMax, contrast) * pow(midIn, contrast * shoulder) * midOut)) /
		(pow(hdrMax, contrast * shoulder) * midOut - pow(midIn, contrast * shoulder) * midOut)) /
		(pow(midIn, contrast * shoulder) * midOut));
}

// General tonemapping operator, build 'c' term.
float ColToneC(float hdrMax, float contrast, float shoulder, float midIn, float midOut)
{
	return (pow(hdrMax, contrast * shoulder) * pow(midIn, contrast) - pow(hdrMax, contrast) * pow(midIn, contrast * shoulder) * midOut) /
		(pow(hdrMax, contrast * shoulder) * midOut - pow(midIn, contrast * shoulder) * midOut);
}

// General tonemapping operator, p := {contrast,shoulder,b,c}.
float ColTone(float x, float4 p)
{
	float z = pow(x, p.r);
	return z / (pow(z, p.g) * p.b + p.a);
}

float3 AMDTonemapper(float3 color)
{
	static const float hdrMax = 16.0; // How much HDR range before clipping. HDR modes likely need this pushed up to say 25.0.
	static const float contrast = 1.8; // Use as a baseline to tune the amount of contrast the tonemapper has.
	static const float shoulder = 1.0; // Likely don’t need to mess with this factor, unless matching existing tonemapper is not working well..
	static const float midIn = 0.20; // most games will have a {0.0 to 1.0} range for LDR so midIn should be 0.18.
	static const float midOut = 0.21; // Use for LDR. For HDR10 10:10:10:2 use maybe 0.18/25.0 to start. For scRGB, I forget what a good starting point is, need to re-calculate.

	const float b = ColToneB(hdrMax, contrast, shoulder, midIn, midOut);
	const float c = ColToneC(hdrMax, contrast, shoulder, midIn, midOut);

#define EPS 1e-6f
	float peak = max(color.r, max(color.g, color.b));
	peak = max(EPS, peak);

	float3 ratio = color / peak;
	peak = ColTone(peak, float4(contrast, shoulder, b, c));
	// then process ratio

	// probably want send these pre-computed (so send over saturation/crossSaturation as a constant)
	static const float crosstalk = 4.0; // controls amount of channel crosstalk
	static const float saturation = 1.4; // full tonal range saturation control
	static const float crossSaturation = contrast * 16.0; // crosstalk saturation

	float white = 1.0;

	// wrap crosstalk in transform
	ratio = pow(abs(ratio), saturation / crossSaturation);
	ratio = lerp(ratio, white, pow(peak, crosstalk));
	ratio = pow(abs(ratio), crossSaturation);

	// then apply ratio to peak
	color = peak * ratio;
	return color;
}

//--------------------------------------------------------------------------------------
// The tone mapper used in HDRToneMappingCS11
//--------------------------------------------------------------------------------------
float3 DX11DSK(float3 color)
{
	float  MIDDLE_GRAY = 0.72f;
	float  LUM_WHITE = 1.5f;

	// Tone mapping
	color.rgb *= MIDDLE_GRAY;
	color.rgb *= (1.0f + color / LUM_WHITE);
	color.rgb /= (1.0f + color);

	return color;
}

//--------------------------------------------------------------------------------------
// Reinhard
//--------------------------------------------------------------------------------------
float3 Reinhard(in float3 color)
{
	return color / (1 + color);
}

//--------------------------------------------------------------------------------------
// Hable's filmic
//--------------------------------------------------------------------------------------
float3 Uncharted2TonemapOp(in float3 x)
{
	static const float A = 0.15;
	static const float B = 0.50;
	static const float C = 0.10;
	static const float D = 0.20;
	static const float E = 0.02;
	static const float F = 0.30;

	return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

float3 Uncharted2Tonemap(in float3 color)
{
	static const float W = 11.2;
	return Uncharted2TonemapOp(2.0 * color) / Uncharted2TonemapOp(W);
}

float3 ApplyGamma(in float3 color)
{
	return pow(color.rgb, 1.0f / 1.4f);
}

float3 Saturation(in float3 In, in float value)
{
	float luma = dot(In, float3(0.2126729, 0.7151522, 0.0721750));
	return luma.xxx + value.xxx * (In - luma.xxx);
}

//--------------------------------------------------------------------------------------
// https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
//--------------------------------------------------------------------------------------
float3 ACESFilm(float3 x)
{
	static const float a = 2.51f;
	static const float b = 0.03f;
	static const float c = 2.43f;
	static const float d = 0.59f;
	static const float e = 0.14f;
	return (x * (a * x + b)) / (x * (c * x + d) + e);
}

cbuffer ScreenSizeCB : register(b0)
{
	int2 screenSize;
	int mode;
	float saturation;
	float2 sunPos;
};

// god rays samples
#define NUM_SAMPLES 80

float4 PS(PostProcessVertex i) : SV_Target
{
	i.texCoord.y = 1 - i.texCoord.y;
	
	float3 color = _texture.Sample(textureSampler, i.texCoord).xyz * 0.82;
	color = Saturation(color, saturation);
	
	float3 tonemapped = color;
	
	switch (mode)
	{
		case 0: tonemapped = ACESFilm(color);  			break;
		case 1: tonemapped = AMDTonemapper(color);  	break;
		case 2: tonemapped = Uncharted2Tonemap(color);  break;
		case 3: tonemapped = Reinhard(color);  			break;
		case 4: tonemapped = DX11DSK(color);  			break;
	}
	
	const float Exposure = 0.3f;
	const float Decay    = 0.96815;
	const float Density  = 0.926;
	const float Weight   = 0.587;
	
	float2 texCoord = i.texCoord;
	// Calculate vector from pixel to light source in screen space.
	float2 deltaTexCoord = (i.texCoord - sunPos);
	// Divide by number of samples and scale by control factor.
	deltaTexCoord *= 1.0f / NUM_SAMPLES * Density;
	// Store initial sample.
	float3 godRaysColor = float3(0.0, 0.0, 0.0); // tex2D(frameSampler, texCoord);
	// Set up illumination decay factor.
	float illuminationDecay = 1.0f;
	// Evaluate summation from Equation 3 NUM_SAMPLES iterations.
	for (int j = 0; j < NUM_SAMPLES; j++) 
	{     
		// Step sample location along ray.
		texCoord -= deltaTexCoord;
		// Retrieve sample at new location.
		float hasSun = distance(texCoord, sunPos) < 0.038 ? 0.85 : 0;
		
		float3 _sample = depthTexture.Sample(depthSampler, texCoord).r > .9992 ?
				       float3(0.08 + hasSun, 0.065 + hasSun, 0.05 + hasSun) : 
				       float3(0.0, 0.0, 0.0); 
	
		// Apply sample attenuation scale/decay factors.
		_sample *= illuminationDecay * Weight;
		// Accumulate combined color.
		godRaysColor += _sample;     
		// Update exponential decay factor.
		illuminationDecay *= Decay;
	}
	
	#define GAMMA 1.4f
	tonemapped = pow(tonemapped, float3(1.0f, 1.0f, 1.0f) / float3(GAMMA, GAMMA, GAMMA));
	//tonemapped = sqrt(tonemapped); // gamma 2.0
	float3 ssao = ssaoTexture.Sample(textureSampler2, i.texCoord).xyz;
	tonemapped *= ssao;
	float4 bloom = bloomTexture.Sample(textureSampler1, i.texCoord);

	return float4(tonemapped.xyz + (godRaysColor * Exposure * 0.5f), 1.0f) + bloom;
	// float hasSun = distance(i.texCoord, sunPos) < 0.058 ? 0.90 : 0;
	// return float4(color.x, color.y, color.z, 1.0) + float4(hasSun, 0.0, 0.0, 0.0);
}