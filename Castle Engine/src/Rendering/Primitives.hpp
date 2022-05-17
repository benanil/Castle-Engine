#pragma once

#include "../Rendering.hpp"
#include "../DirectxBackend.hpp"
#include "../Engine.hpp"
#include <stdexcept>
#include "LineDrawer.hpp"
#include "../Math.hpp"
#include "assimp/scene.h"
#include <fstream>
#include "../Math.hpp"
#include "../Extern/Quicklz/quicklz.h"

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

	std::string name;

	DXBuffer* vertexBuffer;
	DXBuffer* indexBuffer;

	uint16_t materialIndex;

	CMath::AABB aabb;

	DXGI_FORMAT indiceFormat = DXGI_FORMAT_R32_UINT;

	void SaveMesh(std::ofstream& stream)
	{
		qlz_state_compress* state_compress = new qlz_state_compress();
		// write index, vertex count
		stream.write((char*)&vertexCount, sizeof(uint32_t));
		stream.write((char*)&indexCount, sizeof(uint32_t));
		
		// compress if only mesh vertices is bigger than 300 byte
		if (vertexCount * sizeof(Vertex) > 300) {
			// allocate buffer for storing compressed vertice/indice data
			char* buff = (char*)malloc(vertexCount * sizeof(Vertex) + 64);
			// compress vertices to buffer
			size_t vertexCompSize = qlz_compress(vertices, buff, sizeof(Vertex) * vertexCount, state_compress);
			// write compressed size of the vertices
			stream.write((char*)&vertexCompSize, sizeof(size_t));
			stream.write((char*)buff, vertexCompSize); // write compressed data to file
			delete state_compress; // vertex compressing end

			// check if compressed index buffer is bigger than vertex buffer which is not looking posible but anyways. expand buffer for bigger indices
			if (vertexCount * sizeof(Vertex) < indexCount * sizeof(uint32_t)) {
				buff = (char*)realloc(buff, indexCount * sizeof(uint32_t));
			}

			state_compress = new qlz_state_compress(); // create new compress state
			// compress index data to buffer
			size_t indexCompSize = qlz_compress(indices, buff, sizeof(uint32_t) * indexCount, state_compress);
			stream.write((char*)&indexCompSize, sizeof(size_t)); // write index compressed size to file
			stream.write((char*)buff, indexCompSize); // write compressed indices to file
			delete state_compress; // index compressing end
			free(buff);
		}
		else {
			stream.write((char*)&vertices, vertexCount * sizeof(Vertex));
			stream.write((char*)&indices , indexCount  * sizeof(uint32_t));
		}
		stream.write((char*)&materialIndex, sizeof(uint16_t));
		
		uint32_t nameSize = name.size();
		stream.write((char*)&nameSize, sizeof(uint32_t));
		stream.write(name.c_str(), name.size());
	}

	void LoadMesh(std::ifstream& stream)
	{
		// read index, vertex count
		stream.read((char*)&vertexCount, sizeof(uint32_t));
		stream.read((char*)&indexCount, sizeof(uint32_t));
		
		// decompress if only mesh vertices is bigger than 300 byte
		if (vertexCount * sizeof(Vertex) > 300) {
			// start reading vertex data:
			// create state for vertices
			qlz_state_decompress* state_decompress = new qlz_state_decompress();
			// allocate required space for vertices and indices
			vertices = (Vertex*)malloc(vertexCount * sizeof(Vertex));
			indices = (uint32_t*)malloc(indexCount * sizeof(uint32_t));

			size_t vertexCompSize = 0; // get compressed size of vertices
			stream.read((char*)&vertexCompSize, sizeof(size_t));

			// allocate buffer for reading compressed data from file
			char* buff = (char*)malloc(vertexCompSize);
			stream.read(buff, vertexCompSize); // read compressed vertex data
			// decompress vertex data to vertices
			qlz_decompress(buff, vertices, state_decompress);
			delete state_decompress; // reading vertex data end

			// start reading index data:
			size_t indexCompSize = 0; // get compressed indices size
			stream.read((char*)&indexCompSize, sizeof(size_t));

			if (indexCompSize > vertexCompSize)
				buff = (char*)realloc(buff, indexCompSize);
			// read compressed indices data
			stream.read((char*)buff, indexCompSize);

			state_decompress = new qlz_state_decompress();
			qlz_decompress(buff, indices, state_decompress);
			delete state_decompress; // index decompressing end
			free(buff);
		}
		else {
			stream.read((char*)vertices, vertexCount * sizeof(Vertex));
			stream.read((char*)indices , indexCount * sizeof(uint32_t));
		}
		
		stream.read((char*)&materialIndex, sizeof(uint16_t));
		
		uint32_t nameSize = 0;
		stream.read((char*)&nameSize, sizeof(uint32_t));
		name.resize(nameSize);
		stream.read(&name[0], nameSize);

		CreateAABB();
		CreateDXBuffers();
	}

	SubMesh() {};
	~SubMesh() {  };
	SubMesh(const aiMesh& aimesh, bool isSponza)
		: //name(std::string("name")),// name(aimesh.mName.C_Str()),
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
		if (indexCount > UINT32_MAX) return (DXGI_FORMAT)-1;
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
		d3d11DevCon->IASetIndexBuffer(indexBuffer, indiceFormat, 0);
		//Set the vertex buffer
		UINT stride = sizeof(Vertex), offset = 0;
		d3d11DevCon->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
		d3d11DevCon->DrawIndexed(indexCount, 0, 0);
	}

	void Dispose()
	{
		DX_RELEASE(vertexBuffer);
		DX_RELEASE(indexBuffer);
	}
};