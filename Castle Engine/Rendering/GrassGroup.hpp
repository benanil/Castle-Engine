#pragma once
#include "Terrain.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "spdlog/spdlog.h"

class GrassGroup
{
public:
	ID3D11ShaderResourceView* srv;
	ID3D11Buffer* structuredBuffer;

public:
	~GrassGroup() { DX_RELEASE(structuredBuffer); DX_RELEASE(srv);  }

	GrassGroup(XMMATRIX* _positions, ID3D11Device* device)
	{
		DX_CREATE(D3D11_BUFFER_DESC, inputDesc);
		inputDesc.Usage = D3D11_USAGE_DEFAULT;
		inputDesc.ByteWidth = TERRAIN_GRASS_PER_CHUNK * sizeof(XMMATRIX);
		inputDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		inputDesc.CPUAccessFlags = 0;
		inputDesc.StructureByteStride = sizeof(XMMATRIX);
		inputDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		DX_CREATE(D3D11_SUBRESOURCE_DATA, vinitData);
		vinitData.pSysMem = _positions;

		DX_CHECK(device->CreateBuffer(&inputDesc, &vinitData, &structuredBuffer), "structured buffer creation failed");

		DX_CREATE(D3D11_SHADER_RESOURCE_VIEW_DESC, srvDesc);
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		srvDesc.BufferEx.NumElements = TERRAIN_GRASS_PER_CHUNK;

		DX_CHECK(device->CreateShaderResourceView(structuredBuffer, &srvDesc, &srv), "Structured Buffer SRV creation failed!");
	}
};

