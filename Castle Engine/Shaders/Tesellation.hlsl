#define NOISE_SIMPLEX_1_DIV_289 0.00346020761245674740484429065744f
#define OUTPUT_PATCH_SIZE 3

// noise from fadookie: https://gist.github.com/fadookie/25adf86ae7e2753d717c
float mod289(float x)   { return x - floor(x * NOISE_SIMPLEX_1_DIV_289) * 289.0;}
float2 mod289(float2 x) { return x - floor(x * NOISE_SIMPLEX_1_DIV_289) * 289.0;}
float3 mod289(float3 x) { return x - floor(x * NOISE_SIMPLEX_1_DIV_289) * 289.0;}
float4 mod289(float4 x) { return x - floor(x * NOISE_SIMPLEX_1_DIV_289) * 289.0; }
float permute(float x)  { return mod289(x*x*34.0 + x);}
float3 permute(float3 x) { return mod289( x*x*34.0 + x ); }
float4 permute(float4 x) { return mod289(x*x*34.0 + x );}
float4 grad4(float j, float4 ip) {
	const float4 ones = float4(1.0, 1.0, 1.0, -1.0);
	float4 p, s;
	p.xyz = floor( frac(j * ip.xyz) * 7.0) * ip.z - 1.0;
	p.w = 1.5 - dot( abs(p.xyz), ones.xyz );
	p.xyz -= sign(p.xyz) * (p.w < 0);
	return p;
}
float snoise(float2 v) {
	const float4 C = float4( 0.211324865405187, 0.366025403784439, -0.577350269189626,0.024390243902439 );
	float2 i = floor( v + dot(v, C.yy) );
	float2 x0 = v - i + dot(i, C.xx);
	int2 i1 = (x0.x > x0.y) ? float2(1.0, 0.0) : float2(0.0, 1.0);
	float4 x12 = x0.xyxy + C.xxzz;
	x12.xy -= i1; i = mod289(i); // Avoid truncation effects in permutation
	float3 p = permute( permute( i.y + float3(0.0, i1.y, 1.0 ) ) + i.x + float3(0.0, i1.x, 1.0 ));
	float3 m = max( 0.5 - float3( dot(x0, x0), dot(x12.xy, x12.xy),dot(x12.zw, x12.zw) ),0.0 );
	m = m*m ; m = m*m ;
	float3 x = 2.0 * frac(p * C.www) - 1.0;
	float3 h = abs(x) - 0.5;
	float3 ox = floor(x + 0.5);
	float3 a0 = x - ox;
	m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );
	float3 g;
	g.x = a0.x * x0.x + h.x * x0.y;
	g.yz = a0.yz * x12.xz + h.yz * x12.yw;
	return 130.0 * dot(m, g);
}

struct Vertex {
    float4 position : SV_POSITION;
    float3 normal   : NORMAL;
};

struct ConstantOutputType {
    float edges[3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};

// VERTEX SHADER
Vertex VS(float4 Pos : POSITION, float4 normal : NORMAL)
{
	Vertex o;
    o.position = Pos;
    o.normal = normal.xyz;
    return o;
}


cbuffer TessellationBuffer : register(b1) {
	float tessellationAmount;
	float3 camPos;
};

// HULL SHADER
ConstantOutputType ColorPatchConstantFunction(
	InputPatch<Vertex, OUTPUT_PATCH_SIZE> inputPatch, uint patchId : SV_PrimitiveID)
{
    ConstantOutputType output;
	
	output.edges[0] = max(2000 - distance(inputPatch[0].position, camPos), 1) / 1000;
	output.edges[1] = max(2000 - distance(inputPatch[1].position, camPos), 1) / 1000;
	output.edges[2] = max(2000 - distance(inputPatch[2].position, camPos), 1) / 1000;
	output.inside = output.edges[0] + output.edges[1] + output.edges[2] * 0.333;
	return output;	
}					

[domain("tri")] [partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)] // [maxtessfactor(32.0)] 
[patchconstantfunc("ColorPatchConstantFunction")]
Vertex HS( InputPatch<Vertex, 3> patch, 
    uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
	Vertex output;
    output.position = patch[pointId].position;
    output.normal = patch[pointId].normal;
    return output;
}

// DOMAIN SHADER
cbuffer cbPerObject : register(b0)
{
    float4x4 MVP;
	float noiseScale, bias, noiseHeight, _padding; // 100, 0, 30
	float3x4 Model; // we are not using it for now [depricated]
};

[domain("tri")]
Vertex DS(ConstantOutputType input, float3 uvwCoord : SV_DomainLocation, 
	const OutputPatch<Vertex, OUTPUT_PATCH_SIZE> patch)
{
    float3 vertexPosition;
	Vertex output;

	// Determine the position of the new vertex.note texcoord is same
    vertexPosition = uvwCoord.x * patch[0].position.xyz + 
				     uvwCoord.y * patch[1].position.xyz + 
					 uvwCoord.z * patch[2].position.xyz;

	float reallNoise = noiseScale * 0.1;
	
	float displacement = snoise(vertexPosition.xz * reallNoise) ; // todo add uniform scale, and bias
	vertexPosition -= float3(0, 1, 0) * displacement * noiseHeight;

	float left  = snoise(vertexPosition.xz * reallNoise  + (float2(-1, 0) * reallNoise ) );
	float right = snoise(vertexPosition.xz * reallNoise  + (float2( 1, 0) * reallNoise ) );
	float up    = snoise(vertexPosition.xz * reallNoise  + (float2( 0, 1) * reallNoise ) );
	float down  = snoise(vertexPosition.xz * reallNoise  + (float2( 0,-1) * reallNoise ) );
	
	output.normal = normalize(float3(left - right, 2.0, up - down));
	output.normal.y -= displacement * 0.5;
	// Calculate the position of the new vertex against the world, view, and projection matrices.
	output.position = mul(float4(vertexPosition, 1.0f), MVP);

	return output;  
}

// pixel shader
float4 PS(Vertex i) : SV_TARGET
{
    float3 l = float3(sin(0.2f),cos(0.2f), 0);
    float ndl = max(dot(i.normal, l),0.2f);
    float4 color = float4(0.3, 0.7, 0.2, 0) * ndl;
    color.a = 1;
	color = float4(0, 0, 0, 1); // FIX MEE!
    return color;
}

