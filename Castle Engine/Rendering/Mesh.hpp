#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>       
#include <assimp/postprocess.h> 
#include <glm/glm.hpp>
#include <cstdint>
#include "../Helper.hpp"
#include "../Engine.hpp"
#include <array>
#include <utility>
#include <vector>
#include "Texture.hpp"

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 texCoord;

	static ID3D11InputLayout* GetLayout(DXBlob* VS_Buffer)
	{
		ID3D11InputLayout* vertLayout;

		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 , D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL"  , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT   , 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		auto device = Engine::GetDevice();
		device->CreateInputLayout(layout, 3, VS_Buffer->GetBufferPointer(),
			VS_Buffer->GetBufferSize(), &vertLayout);

		return vertLayout;
	}

	Vertex() {}
	Vertex(const glm::vec3& _pos, const glm::vec2& _texCoord)
		:
		pos(_pos), texCoord(_texCoord)
	{}
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

	uint32_t materialIndex;

	SubMesh(const aiMesh& aimesh) : name(aimesh.mName.C_Str())
	{
		vertexCount = aimesh.mNumVertices;
		indexCount = aimesh.mNumFaces * 3;

		vertices = (Vertex*)malloc(sizeof(Vertex) * vertexCount);
		indices = (uint32_t*)malloc(sizeof(uint32_t) * indexCount);

		for (uint32_t i = 0; i < vertexCount; ++i) // +1 iter for jumping texcoord.y
		{
			memcpy(&vertices[i], &aimesh.mVertices[i], sizeof(glm::vec3));
			vertices[i].normal.x = aimesh.mNormals[i].x;
			vertices[i].normal.y = aimesh.mNormals[i].y;
			vertices[i].normal.z = aimesh.mNormals[i].z;

			vertices[i].texCoord.x = aimesh.mTextureCoords[0][i].x;
			vertices[i].texCoord.y = aimesh.mTextureCoords[0][i].y;
		}

		DXDevice* d3d11Device = Engine::GetDevice();
		DXDeviceContext* d3d11DevCon = Engine::GetDeviceContext();

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

		for (uint32_t i = 0; i < aimesh.mNumFaces; i++)
		{
			indices[i * 3 + 0] = aimesh.mFaces[i].mIndices[0];
			indices[i * 3 + 1] = aimesh.mFaces[i].mIndices[1];
			indices[i * 3 + 2] = aimesh.mFaces[i].mIndices[2];
		}

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
		d3d11DevCon->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		//Set the vertex buffer
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		d3d11DevCon->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
		d3d11DevCon->DrawIndexed(indexCount, 0, 0);
	}

	void Dispose()
	{
		indexBuffer->Release();
		vertexBuffer->Release();
	}
};

struct Mesh
{
	SubMesh* subMeshes;
	std::vector<Texture*> textures;
	uint16_t subMeshCount;

	void Draw(DXDeviceContext* deviceContext)
	{
		for (uint16_t i = 0; i < subMeshCount; i++)
		{
			textures[std::min((uint32_t)textures.size()-1, subMeshes[i].materialIndex)]->Bind(deviceContext);
			subMeshes[i].Draw(deviceContext);
		}
	}

	void Dispose()
	{
		for (uint16_t i = 0; i < subMeshCount; i++)
		{
			subMeshes[i].Dispose();
		}
		delete subMeshes;
		subMeshes = nullptr;
	}
};

namespace MeshLoader
{
	static inline D3D11_TEXTURE_ADDRESS_MODE AssimpToD3D11_Wrap(const aiTextureMapMode& aimode)
	{
		switch (aimode)
		{
		case aiTextureMapMode_Wrap:   return D3D11_TEXTURE_ADDRESS_WRAP;
		case aiTextureMapMode_Clamp:  return D3D11_TEXTURE_ADDRESS_CLAMP;
		case aiTextureMapMode_Decal:  return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
		case aiTextureMapMode_Mirror: return D3D11_TEXTURE_ADDRESS_MIRROR;
		default: D3D11_TEXTURE_ADDRESS_WRAP;
		}
	}
	
	static Mesh* LoadMesh(const std::string& path)
	{
		Mesh* mesh = new Mesh();

		Assimp::Importer importer;
		static const uint32_t flags = aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded;
		const aiScene* scene = importer.ReadFile(path, flags);

		if (!scene)
		{
			DX_CHECK(-1, (path + std::string("MESH LOADING FAILED!")).c_str());
			return nullptr;
		}

		mesh->subMeshes = (SubMesh*)malloc(sizeof(SubMesh) * scene->mNumMeshes);
		mesh->subMeshCount = scene->mNumMeshes;

		for (uint16_t i = 0; i < scene->mNumMaterials; i++)
		{
			const auto& material = scene->mMaterials[i];
			
			aiString aiPath;
			aiTextureMapMode aiWrapmode = aiTextureMapMode::aiTextureMapMode_Wrap;
		
			if (material->GetTexture(aiTextureType::aiTextureType_DIFFUSE, 0, &aiPath, 
				nullptr, nullptr, nullptr, nullptr, &aiWrapmode) == 0)
			{
				char cPath[] = "Models\\\0                                                                 ";
				const char* cResult = strcat(cPath, aiPath.C_Str());
				mesh->textures.push_back(new Texture(cResult, AssimpToD3D11_Wrap(aiWrapmode)));
			}
		}

		for (uint16_t i = 0; i < scene->mNumMeshes; i++)
		{
			mesh->subMeshes[i] = SubMesh(*scene->mMeshes[i]);
			mesh->subMeshes[i].materialIndex = scene->mMeshes[i]->mMaterialIndex;
		}
		return mesh;
	}
}
