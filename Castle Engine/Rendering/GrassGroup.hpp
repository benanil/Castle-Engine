#pragma once
#include "Terrain.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "spdlog/spdlog.h"

class GrassGroup
{
public:
	~GrassGroup() {
		DX_RELEASE(vertexBuffer); DX_RELEASE(indexBuffer);
	}

	GrassGroup(glm::vec3* _positions, ID3D11Device* device)
	{
		DX_CREATE(D3D11_BUFFER_DESC, vertexBufferDesc);
		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(GrassVertex) * TERRAIN_GRASS_PER_CHUNK * 4;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		GrassVertex* vertices = CreateVertices(_positions);
		DX_CREATE(D3D11_SUBRESOURCE_DATA, vinitData);
		vinitData.pSysMem = vertices;
		if (FAILED(device->CreateBuffer(&vertexBufferDesc, &vinitData, &vertexBuffer))) {
			assert(0, "Constant Buffer Creation Failed!");
		}
		
		DX_CREATE(D3D11_BUFFER_DESC, indexBufferDesc);
		indexBufferDesc.Usage     = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(uint32_t) * TERRAIN_GRASS_PER_CHUNK * 6;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		uint32_t* indices = CreateIndices();

		vinitData.pSysMem = indices;
		
		if (FAILED(device->CreateBuffer(&indexBufferDesc, &vinitData, &indexBuffer))) {
			assert(0, "Constant Buffer Creation Failed!");
		}
		//free(vertices); free(indices);
	}

	ID3D11Buffer* vertexBuffer, *indexBuffer;

private:
	GrassVertex* CreateVertices(glm::vec3* points)
	{
		GrassVertex* result = (GrassVertex*)malloc(sizeof(GrassVertex) * TERRAIN_GRASS_PER_CHUNK * 4);
		memset(result, 0, sizeof(uint32_t) * TERRAIN_GRASS_PER_CHUNK * 4);

		for (uint32_t i = 0, p = 0; i < TERRAIN_GRASS_PER_CHUNK * 4; i += 4, ++p) {
			
			result[i + 0].position.x = points[p].x + 0.0f;
			result[i + 0].position.y = points[p].y + 0.0f;
			result[i + 0].position.z = points[p].z + 0.0f;

			result[i + 1].position.x = points[p].x + 0.0f;
			result[i + 1].position.y = points[p].y + 5.0f;
			result[i + 1].position.z = points[p].z;

			result[i + 2].position.x = points[p].x + 5.0f;
			result[i + 2].position.y = points[p].y + 0.0f;
			result[i + 2].position.z = points[p].z;

			result[i + 3].position.x = points[p].x + 5.0f;
			result[i + 3].position.y = points[p].y + 5.0f;
			result[i + 3].position.z = points[p].z;

			result[i + 0].uv = glm::vec2(0.0f, 1.0f);
			result[i + 1].uv = glm::vec2(0.0f, 0.0f);
			result[i + 2].uv = glm::vec2(1.0f, 1.0f);
			result[i + 3].uv = glm::vec2(1.0f, 0.0f);
		}
		return result;
	}
	uint32_t* CreateIndices()
	{
		uint32_t* result = (uint32_t*)malloc(sizeof(uint32_t) * TERRAIN_GRASS_PER_CHUNK * 6);
		memset(result, 0, sizeof(uint32_t) * TERRAIN_GRASS_PER_CHUNK * 6);
		
		for (uint32_t i = 0; i < TERRAIN_GRASS_PER_CHUNK * 6; i += 6) {
			result[i + 0] = i + 0; 
			result[i + 1] = i + 1; 
			result[i + 2] = i + 2;
			result[i + 3] = i + 1;
			result[i + 4] = i + 3;
			result[i + 5] = i + 2;
		}
		return result;
	}
};

