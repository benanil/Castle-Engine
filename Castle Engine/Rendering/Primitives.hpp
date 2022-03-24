#pragma once

#include "../Rendering.hpp"
#include "../DirectxBackend.hpp"
#include "../Engine.hpp"
#include <stdexcept>
#include "LineDrawer.hpp"
#include "../Math.hpp"
#include "assimp/scene.h"

#ifndef CB_GlobalDefined
# define CB_GlobalDefined
  struct cbGlobal
  {
	float sunAngle;
	glm::vec3 ambientColor;  // 16
	glm::vec3 sunColor;      
	float additionalData;      // 32
	glm::vec3 viewPos;
	float ambientStength; // 48
  };
#endif

struct cbPerObject
{
	XMMATRIX  MVP;
	XMMATRIX  Model;
};

// pbr vertex
struct Vertex
{
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 texCoord;
	glm::vec3 tangent;
};

struct SubMesh
{
	Vertex* vertices;
	uint32_t* indices;
	
	uint32_t indexCount;
	uint32_t vertexCount;
	
	const char* name;
	
	DXBuffer* vertexBuffer;
	DXBuffer* indexBuffer;
	
	uint16_t materialIndex;
	
	CMath::AABB aabb;

	DXGI_FORMAT indiceFormat = DXGI_FORMAT_R32_UINT;

	SubMesh(){};
	~SubMesh() { Dispose(); };
	SubMesh(const aiMesh& aimesh, bool isSponza) 
	:	name(aimesh.mName.C_Str()),
		vertexCount(aimesh.mNumVertices),
		indexCount(aimesh.mNumFaces * 3)
	{			
		vertices = (Vertex*)malloc(sizeof(Vertex) * vertexCount);
		indices = (uint32_t*)malloc(sizeof(uint32_t) * indexCount);
		
		for (uint32_t i = 0; i < vertexCount; ++i) 
		{
			vertices[i].pos.x = aimesh.mVertices[i].x;
			vertices[i].pos.y = aimesh.mVertices[i].y;
			vertices[i].pos.z = aimesh.mVertices[i].z;
			
			vertices[i].normal.x = aimesh.mNormals[i].x;
			vertices[i].normal.y = aimesh.mNormals[i].y;
			vertices[i].normal.z = aimesh.mNormals[i].z;
			
			vertices[i].texCoord.x = aimesh.mTextureCoords[0][i].x;
			vertices[i].texCoord.y = aimesh.mTextureCoords[0][i].y;
			
			vertices[i].tangent.x = aimesh.mBitangents[i].x;
			vertices[i].tangent.y = aimesh.mBitangents[i].y;
			vertices[i].tangent.z = aimesh.mBitangents[i].z;
		}
		
		if (isSponza)
		{
			for (uint32_t i = 0; i < vertexCount; ++i)
			{
				vertices[i].pos.x *= 0.0125f;
				vertices[i].pos.y *= 0.0125f;
				vertices[i].pos.z *= 0.0125f;
			}
		}

		for (uint32_t i = 0; i < aimesh.mNumFaces; i++)
		{
			indices[i * 3 + 0] = aimesh.mFaces[i].mIndices[0];
			indices[i * 3 + 1] = aimesh.mFaces[i].mIndices[1];
			indices[i * 3 + 2] = aimesh.mFaces[i].mIndices[2];
		}

		CreateAABB();
		CreateDXBuffers();
	}

	void CreateAABB()
	{
		aabb.min = glm::vec3(+999999.0f);
		aabb.max = glm::vec3(-999999.0f);

		for (uint32_t i = 0; i < vertexCount; ++i)
		{
			aabb.min = glm::min(aabb.min, vertices[i].pos + glm::vec3(30000.0f));
			aabb.max = glm::max(aabb.max, vertices[i].pos + glm::vec3(30000.0f));
		}

		aabb.min -= glm::vec3(30000.0f);
		aabb.max -= glm::vec3(30000.0f);
	}
	
	static __forceinline DXGI_FORMAT ChoseIndiceFormat(uint64_t indexCount)
	{
		return DXGI_FORMAT_R32_UINT;
		if 		(indexCount > UINT32_MAX) return (DXGI_FORMAT)-1;
		else if (indexCount > UINT16_MAX) return DXGI_FORMAT_R32_UINT;
		else if (indexCount > UINT8_MAX)  return DXGI_FORMAT_R16_UINT;
		else return DXGI_FORMAT_R8_UINT;
	}
	
	void CreateDXBuffers()
	{
		indiceFormat = ChoseIndiceFormat(indexCount);
		DXDevice* d3d11Device = DirectxBackend::GetDevice();
		// create vertex buffer		
		DX_CREATE(D3D11_BUFFER_DESC, vertexBufferDesc);
		
		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(Vertex) * vertexCount;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		
		DX_CREATE(D3D11_SUBRESOURCE_DATA, vertexBufferData);
		
		vertexBufferData.pSysMem = vertices;
		DX_CHECK(
			d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &vertexBuffer), "vertex buffer creation failed!"
		);
		
		// create index buffer
		DX_CREATE(D3D11_BUFFER_DESC, indexDesc);
		indexDesc.Usage = D3D11_USAGE_DEFAULT;
		indexDesc.ByteWidth = sizeof(uint32_t) * indexCount;
		indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexDesc.CPUAccessFlags = 0;
		indexDesc.MiscFlags = 0;
		
		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = indices;
		d3d11Device->CreateBuffer(&indexDesc, &initData, &indexBuffer);
	}
	
	void Draw(DXDeviceContext* d3d11DevCon)
	{
		d3d11DevCon->IASetIndexBuffer(indexBuffer, indiceFormat , 0);
		//Set the vertex buffer
		UINT stride = sizeof(Vertex), offset = 0;
		d3d11DevCon->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
		d3d11DevCon->DrawIndexed(indexCount, 0, 0);
	}
	
	void Dispose()
	{
		// DX_RELEASE(indexBuffer)
		// DX_RELEASE(vertexBuffer)
	}
};