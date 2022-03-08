
struct FXAAVertex
{
	float4 pos   : SV_POSITION;
	centroid noperspective float2 texCoord : TEXCOORD;
};

cbuffer ScreenSizeCB : register(b0)
{
	int2 screenSize; int padding, padding1;
};

Texture2D _texture  : register(t0);
SamplerState texSampler : register(s0);

FXAAVertex VS(uint id : SV_VERTEXID)
{
	FXAAVertex o;
	o.pos.x = (float)(id / 2) * 4 - 1;
	o.pos.y = (float)(id % 2) * 4 - 1;
	o.pos.z = 0.0; o.pos.w = 1.0;
	
	o.texCoord.x = (float)(id / 2) * 2.0;
	o.texCoord.y = (float)(id % 2) * 2.0;
	return o;
}

// http://blog.simonrodriguez.fr/articles/2016/07/implementing_fxaa.html

float rgb2luma(half3 rgb)
{
	return sqrt(dot(rgb, half3(0.299, 0.587, 0.114)));
}

half3 TextureOffset(float2 uv, float2 offset, float2 texelSize)
{
	return _texture.Sample(texSampler, uv + (texelSize * offset)).xyz;
}

#define EDGE_THRESHOLD_MIN 0.0312
#define EDGE_THRESHOLD_MAX 0.125
#define ITERATIONS 12
#define SUBPIXEL_QUALITY 0.75f

half4 PS(FXAAVertex i) : SV_Target
{
	half3 colorCenter = _texture.Sample(texSampler, i.texCoord).xyz;
	float2 texelSize = float2(1.0f, 1.0f) / screenSize;
    
	float lumaCenter = rgb2luma(colorCenter);
	
	float lumaDown  = rgb2luma(TextureOffset(i.texCoord, float2( 0,-1), texelSize));
	float lumaUp    = rgb2luma(TextureOffset(i.texCoord, float2( 0, 1), texelSize));
	float lumaLeft  = rgb2luma(TextureOffset(i.texCoord, float2(-1, 0), texelSize));
	float lumaRight = rgb2luma(TextureOffset(i.texCoord, float2( 1, 0), texelSize));
	
	float lumaMin = min(lumaCenter,min(min(lumaDown,lumaUp),min(lumaLeft,lumaRight)));
	float lumaMax = max(lumaCenter,max(max(lumaDown,lumaUp),max(lumaLeft,lumaRight)));
	// Compute the delta.
	float lumaRange = lumaMax - lumaMin;

	// If the luma variation is lower that a threshold (or if we are in a really dark area), we are not on an edge, don't perform any AA.
	if(lumaRange < max(EDGE_THRESHOLD_MIN,lumaMax*EDGE_THRESHOLD_MAX)){
		return half4(colorCenter, 1.0);
	}

	// Query the 4 remaining corners lumas.
	float lumaDownLeft  = rgb2luma(TextureOffset(i.texCoord, float2(-1,-1), texelSize).rgb);
	float lumaUpRight   = rgb2luma(TextureOffset(i.texCoord, float2( 1, 1), texelSize).rgb);
	float lumaUpLeft    = rgb2luma(TextureOffset(i.texCoord, float2(-1, 1), texelSize).rgb);
	float lumaDownRight = rgb2luma(TextureOffset(i.texCoord, float2( 1,-1), texelSize).rgb);

	// Combine the four edges lumas (using intermediary variables for future computations with the same values).
	float lumaDownUp = lumaDown + lumaUp;
	float lumaLeftRight = lumaLeft + lumaRight;

	// Same for corners
	float lumaLeftCorners = lumaDownLeft + lumaUpLeft;
	float lumaDownCorners = lumaDownLeft + lumaDownRight;
	float lumaRightCorners = lumaDownRight + lumaUpRight;
	float lumaUpCorners = lumaUpRight + lumaUpLeft;

	// Compute an estimation of the gradient along the horizontal and vertical axis.
	float edgeHorizontal =  abs(-2.0 * lumaLeft + lumaLeftCorners)  + abs(-2.0 * lumaCenter + lumaDownUp ) * 2.0    + abs(-2.0 * lumaRight + lumaRightCorners);
	float edgeVertical =    abs(-2.0 * lumaUp + lumaUpCorners)      + abs(-2.0 * lumaCenter + lumaLeftRight) * 2.0  + abs(-2.0 * lumaDown + lumaDownCorners);

	// Is the local edge horizontal or vertical ?
	bool isHorizontal = (edgeHorizontal >= edgeVertical);

	// Select the two neighboring texels lumas in the opposite direction to the local edge.
	float luma1 = isHorizontal ? lumaDown : lumaLeft;
	float luma2 = isHorizontal ? lumaUp : lumaRight;
	// Compute gradients in this direction.
	float gradient1 = luma1 - lumaCenter;
	float gradient2 = luma2 - lumaCenter;

	// Which direction is the steepest ?
	bool is1Steepest = abs(gradient1) >= abs(gradient2);

	// Gradient in the corresponding direction, normalized.
	float gradientScaled = 0.25 * max(abs(gradient1),abs(gradient2));

	// Choose the step size (one pixel) according to the edge direction.
	float stepLength = isHorizontal ? texelSize.y : texelSize.x;

	// Average luma in the correct direction.
	float lumaLocalAverage = 0.0;

	if(is1Steepest){
		// Switch the direction
		stepLength = - stepLength;
		lumaLocalAverage = 0.5*(luma1 + lumaCenter);
	} else {
		lumaLocalAverage = 0.5*(luma2 + lumaCenter);
	}

	// Shift UV in the correct direction by half a pixel.
	float2 currentUv = i.texCoord;
	if(isHorizontal){
		currentUv.y += stepLength * 0.5;
	} else {
		currentUv.x += stepLength * 0.5;
	}
	
	// Compute offset (for each iteration step) in the right direction.
	float2 offset = isHorizontal ? float2(texelSize.x,0.0) : float2(0.0, texelSize.y);
	// Compute UVs to explore on each side of the edge, orthogonally. The QUALITY allows us to step faster.
	float2 uv1 = currentUv - offset;
	float2 uv2 = currentUv + offset;

	// Read the lumas at both current extremities of the exploration segment, and compute the delta wrt to the local average luma.
	float lumaEnd1 = rgb2luma(_texture.Sample(texSampler, uv1).rgb);
	float lumaEnd2 = rgb2luma(_texture.Sample(texSampler, uv2).rgb);
	lumaEnd1 -= lumaLocalAverage;
	lumaEnd2 -= lumaLocalAverage;

	// If the luma deltas at the current extremities are larger than the local gradient, we have reached the side of the edge.
	bool reached1 = abs(lumaEnd1) >= gradientScaled;
	bool reached2 = abs(lumaEnd2) >= gradientScaled;
	bool reachedBoth = reached1 && reached2;

	// If the side is not reached, we continue to explore in this direction.
	if(!reached1){
		uv1 -= offset;
	}
	if(!reached2){
		uv2 += offset;
	}   

	static const float Quality[12] = { 0.5, 0.5, 1.0, 1.0, 1.0, 1.5, 1.5, 2.0, 2.0, 2.0, 4.0, 8.0 };

	// If both sides have not been reached, continue to explore.
	if(!reachedBoth){

		for(int i = 2; i < ITERATIONS; i++){
			// If needed, read luma in 1st direction, compute delta.
			if(!reached1){
				lumaEnd1 = rgb2luma(_texture.Sample(texSampler, uv1).rgb);
				lumaEnd1 = lumaEnd1 - lumaLocalAverage;
			}
			// If needed, read luma in opposite direction, compute delta.
			if(!reached2){
				lumaEnd2 = rgb2luma(_texture.Sample(texSampler, uv2).rgb);
				lumaEnd2 = lumaEnd2 - lumaLocalAverage;
			}
			// If the luma deltas at the current extremities is larger than the local gradient, we have reached the side of the edge.
			reached1 = abs(lumaEnd1) >= gradientScaled;
			reached2 = abs(lumaEnd2) >= gradientScaled;
			reachedBoth = reached1 && reached2;

			// If the side is not reached, we continue to explore in this direction, with a variable quality.
			if(!reached1){
				uv1 -= offset * (Quality[i] * texelSize);
			}
			if(!reached2){
				uv2 += offset * (Quality[i] * texelSize);
			}

			// If both sides have been reached, stop the exploration.
			if(reachedBoth){ break;}
		}
	}
	// -- Estimating offset--
	
	// Compute the distances to each extremity of the edge.
	float distance1 = isHorizontal ? (i.texCoord.x - uv1.x) : (i.texCoord.y - uv1.y);
	float distance2 = isHorizontal ? (uv2.x - i.texCoord.x) : (uv2.y - i.texCoord.y);

	// In which direction is the extremity of the edge closer ?
	bool isDirection1 = distance1 < distance2;
	float distanceFinal = min(distance1, distance2);

	// Length of the edge.
	float edgeThickness = (distance1 + distance2);

	// UV offset: read in the direction of the closest side of the edge.
	float pixelOffset = - distanceFinal / edgeThickness + 0.5;

	// Is the luma at center smaller than the local average ?
	bool isLumaCenterSmaller = lumaCenter < lumaLocalAverage;
	// If the luma at center is smaller than at its neighbour, the delta luma at each end should be positive (same variation).
	// (in the direction of the closer side of the edge.)
	bool correctVariation = ((isDirection1 ? lumaEnd1 : lumaEnd2) < 0.0) != isLumaCenterSmaller;
	// If the luma variation is incorrect, do not offset.
	float finalOffset = correctVariation ? pixelOffset : 0.0;
	
	// -- Subpixel antialiasing --
	// Sub-pixel shifting
	// Full weighted average of the luma over the 3x3 neighborhood.
	float lumaAverage = (1.0/12.0) * (2.0 * (lumaDownUp + lumaLeftRight) + lumaLeftCorners + lumaRightCorners);
	// Ratio of the delta between the global average and the center luma, over the luma range in the 3x3 neighborhood.
	float subPixelOffset1 = clamp(abs(lumaAverage - lumaCenter)/lumaRange,0.0,1.0);
	float subPixelOffset2 = (-2.0 * subPixelOffset1 + 3.0) * subPixelOffset1 * subPixelOffset1;
	// Compute a sub-pixel offset based on this delta.
	float subPixelOffsetFinal = subPixelOffset2 * subPixelOffset2 * SUBPIXEL_QUALITY;

	// Pick the biggest of the two offsets.
	finalOffset = max(finalOffset,subPixelOffsetFinal);
	
	// -- FINAL READ --
	// Compute the final UV coordinates.
	float2 finalUv = i.texCoord;
	if(isHorizontal){
		finalUv.y += finalOffset * stepLength;
	} else {
		finalUv.x += finalOffset * stepLength;
	}

	// Read the color at the new UV coordinates, and use it.
	half4 finalColor = _texture.Sample(texSampler, finalUv);
	return finalColor ;
}

