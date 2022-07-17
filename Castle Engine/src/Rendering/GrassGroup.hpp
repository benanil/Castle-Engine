#pragma once
#include "Terrain.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "spdlog/spdlog.h"

class GrassGroup
{
public:
	~GrassGroup() 
	{
		DX_RELEASE(structuredBuffer); 
		DX_RELEASE(srv);  
	}

    // https://stackoverflow.com/questions/60108658/fastest-method-to-calculate-sum-of-all-packed-32-bit-integers-using-avx512-or-av
	static __forceinline int hsum_epi32_sse2(__m128i x) {
		__m128i hi64 = _mm_unpackhi_epi64(x, x); // 3-operand non-destructive AVX lets us save a byte without needing a mov
		__m128i sum64 = _mm_add_epi32(hi64, x);
		__m128i hi32 = _mm_shufflelo_epi16(sum64, _MM_SHUFFLE(1, 0, 3, 2));    // Swap the low two elements
		__m128i sum32 = _mm_add_epi32(sum64, hi32);
		return _mm_extract_epi32(sum32, 0); // SSE4, even though it compiles to movd instead of a literal pextrd r32,xmm,0
	}

	GrassGroup(XMMATRIX* _positions, int* cullResult, ID3D11Device* device)
	{
		cullledMatrixCount = 0;

		for (uint32_t i = 0; i < TERRAIN_TRIANGLE_COUNT; ++i)
		{
			cullledMatrixCount += cullResult[i] * TERRAIN_GRASS_PER_TRIANGLE;
		}
			
		// static const __m128i _mmGrassPerTriangle = _mm_set1_epi32(TERRAIN_GRASS_PER_TRIANGLE);
		// for (int32_t i = 0; i < TERRAIN_TRIANGLE_COUNT; i += 4) {
		// 	cullledMatrixCount += hsum_epi32_sse2(_mm_mul_epi32(_mm_loadu_epi32(&cullResult[i]), _mmGrassPerTriangle));
		// }

		XMMATRIX* culledMatrices = (XMMATRIX*)malloc(sizeof(XMMATRIX) * cullledMatrixCount);
		
		for (int32_t i = 0, currMat = 0; i < TERRAIN_TRIANGLE_COUNT; ++i)
		{
			if (cullResult[i])
			{
				for (int j = 0; j < TERRAIN_GRASS_PER_TRIANGLE; ++j)
				culledMatrices[currMat++] = _positions[i * TERRAIN_GRASS_PER_TRIANGLE + j];
			}
		}

		DX_CREATE(D3D11_BUFFER_DESC, inputDesc);
		inputDesc.Usage = D3D11_USAGE_DEFAULT;
		inputDesc.ByteWidth = cullledMatrixCount * sizeof(XMMATRIX);
		inputDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		inputDesc.CPUAccessFlags = 0;
		inputDesc.StructureByteStride = sizeof(XMMATRIX);
		inputDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		DX_CREATE(D3D11_SUBRESOURCE_DATA, vinitData);
		vinitData.pSysMem = culledMatrices;

		DX_CHECK(device->CreateBuffer(&inputDesc, &vinitData, &structuredBuffer), "structured buffer creation failed");

		DX_CREATE(D3D11_SHADER_RESOURCE_VIEW_DESC, srvDesc);
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		srvDesc.BufferEx.NumElements = cullledMatrixCount;

		DX_CHECK(device->CreateShaderResourceView(structuredBuffer, &srvDesc, &srv), "Structured Buffer SRV creation failed!");
		
		free(culledMatrices);
	}

public:
	ID3D11ShaderResourceView* srv;
	ID3D11Buffer* structuredBuffer;
	uint32_t cullledMatrixCount;
};
