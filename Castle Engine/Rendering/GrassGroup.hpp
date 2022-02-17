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
		free(positions);
		DX_RELEASE(srv);
	}

	GrassGroup(glm::vec3* _positions, ID3D11Device* device) : positions(positions)
	{
		DX_CREATE(D3D11_BUFFER_DESC, instanceBufferDesc);
		instanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		instanceBufferDesc.ByteWidth = sizeof(glm::vec3) * TERRAIN_GRASS_PER_CHUNK;
		instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		DX_CREATE(D3D11_SUBRESOURCE_DATA, vinitData);
		vinitData.pSysMem = _positions;
		if (FAILED(device->CreateBuffer(&instanceBufferDesc, &vinitData, &srv))) {
			assert(0, "Constant Buffer Creation Failed!");
		}
	}
	ID3D11Buffer* srv;
private:
	glm::vec3* positions;
};

