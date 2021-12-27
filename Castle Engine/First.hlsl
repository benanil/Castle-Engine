cbuffer cbPerObject : register(b0)
{
    float4x4 MVP;
    float4x4 Model;
};

cbuffer cbPerMaterial : register(b1)
{
	float shininess;
	float roughness;
	float metallic;
	float padding;
};

cbuffer cbGlobal : register(b2)
{
	float sunAngle;
	float3 ambientColor;  // 16
	float3 sunColor;
	float fogAmount;      // 32
	float3 viewPos;
	float ambientStength; // 48
};

struct VS_OUTPUT
{
    float4 Pos      : SV_POSITION;
    float2 TexCoord : TEXCOORD;
	float3 normal   : NORMAL;
	float3 tangent : TANGENT;
};

Texture2D ObjTexture      : register(t0);   
Texture2D SpecularTexture : register(t1);
Texture2D NormalTexture   : register(t2);

SamplerState ObjSamplerState : register(s0); 
SamplerState SpecularSampler : register(s1); 
SamplerState NormalSampler   : register(s2); 

VS_OUTPUT VS(float4 inPos : POSITION, float2 inTexCoord : TEXCOORD, float4 inNormal : NORMAL, float4 inTangent : TANGENT)
{
    VS_OUTPUT output;

    output.Pos = mul(inPos, MVP);
	output.normal  = mul(inNormal, Model).xyz;
	output.TexCoord = inTexCoord;
	output.tangent = mul(inTangent, Model).xyz;
    return output;
}

#define DX_DEG_TO_RAD 0.01745f
#define DX_RAD_TO_DEG 57.295f
#define PI 3.1415f

// product could be NdV or VdH depending on used technique
float3 fresnel_factor(in float3 F0, in float cosTheta)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float G_schlick(in float roughness, in float NdV, in float NdL)
{
	float k = roughness * roughness * 0.5;
	float V = NdV * (1.0 - k) + k;
	float L = NdL * (1.0 - k) + k;
	return 0.25 / (V * L);
}

float D_blinn(in float roughness, in float NdH)
{
	float m = roughness * roughness;
	float m2 = m * m;
	float n = 2.0 / m2 - 2.0;
	return (n + 2.0) / (2.0 * PI) * pow(NdH, n);
}

float D_beckmann(in float roughness, in float NdH)
{
	float m = roughness * roughness;
	float m2 = m * m;
	float NdH2 = NdH * NdH;
	return exp((NdH2 - 1.0) / (m2 * NdH2)) / (PI * m2 * NdH2 * NdH2);
}

float D_GGX(in float roughness, in float cosLh)
{
	float alpha = roughness * roughness;
	float alphaSq = alpha * alpha;

	float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
	return alphaSq / (PI * denom * denom);
}

// cook-torrance specular calculation                      
float3 cooktorrance_specular(in float NdL, in float NdV, in float NdH, in float3 specular, in float roughness, in float metallic)
{
	// float D = min(D_blinn(roughness, NdH), 1);
	// float D = min(D_beckmann(roughness, NdH), 1);
	float D = D_GGX(roughness, NdH);
	
	float G = G_schlick(roughness, NdV, NdL);
	
	float rim = lerp(1.0 - roughness * metallic * 0.9, 1.0, NdV);
	
	return (1.0 / rim) * specular * G * D;
}

float4 PS(VS_OUTPUT input) : SV_TARGET
{
	float4 albedo = ObjTexture.Sample(ObjSamplerState, input.TexCoord);
	float4 specularTex = SpecularTexture.Sample(SpecularSampler, input.TexCoord) * shininess;
	 
	clip(albedo.a - 0.15f);

	float3 L = float3(0, max(sin(sunAngle * DX_DEG_TO_RAD), 0), cos(sunAngle * DX_DEG_TO_RAD));
	float3 V = normalize(viewPos - input.Pos.xyz);
	float3 H = normalize(V - L);

	//normal map calculation
		float3 normalMap = NormalTexture.Sample(NormalSampler, input.TexCoord).xyz;
		
		//Change normal map range from [0, 1] to [-1, 1]
		normalMap = (2.0f * normalMap) - 1.0f;
		
		//Make sure tangent is completely orthogonal to normal
		input.tangent = normalize(input.tangent - dot(input.tangent, input.normal) * input.normal);
		
		//Create the biTangent
		float3 biTangent = cross(input.normal, input.tangent);
		
		//Create the "Texture Space"
		float3x3 texSpace = float3x3(input.tangent, biTangent, input.normal);
		
		//Convert normal from normal map to texture space and store in input.normal
		float3 normal = normalize(mul(normalMap, texSpace));
	// normal map calculation end

	float ndl = max(0.2f, dot(normal, L));
	float ndv = max(0.0f, dot(normal, V));
	float ndh = max(0.0f, dot(normal, H));
	float hdv = max(0.0f, dot(H, V));

	float3 F0 = float3(0,0,0);
	F0 = lerp(F0, albedo.xyz, metallic);

	float3 specFresnel = fresnel_factor(F0, hdv);
	float3 specular = cooktorrance_specular(ndl, ndv, ndh, specFresnel, max(roughness, 0.001f) , metallic) * ndl * specularTex.xyz;
	
	float3 ambient = ambientColor * ambientStength;

	float4 result = float4(0, 0, 0, 1);
	result.xyz = albedo.xyz * (ndl * sunColor) + specular + ambient;
    
	return result;
}

// sadfasdf