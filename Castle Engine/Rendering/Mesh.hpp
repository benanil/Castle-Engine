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
#include "../ECS/ECS.hpp"
#ifndef NEDITOR
#	include "../Editor/Editor.hpp"
#endif NEDITOR

#include "AssetManager.hpp"
#include "Material.hpp"

#include "Primitives.hpp"
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

	SubMesh(const aiMesh& aimesh) : name(aimesh.mName.C_Str())
	{
		vertexCount = aimesh.mNumVertices;
		indexCount = aimesh.mNumFaces * 3;

		vertices = (Vertex*)malloc(sizeof(Vertex) * vertexCount);
		indices = (uint32_t*)malloc(sizeof(uint32_t) * indexCount);

		for (uint32_t i = 0; i < vertexCount; ++i) // +1 iter for jumping texcoord.y
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


class MeshRenderer : ECS::Component
{
public:
	SubMesh* subMeshes;
	std::vector<Material*> materials;
	uint16_t subMeshCount;

	MeshRenderer() : ECS::Component() {}

	ECS::Entity* GetEntity() { return entity; }
	void SetEntity(ECS::Entity* _entity) { entity = _entity; }

	void Update(const float& deltaTime) {}
	void OnEditor()
	{
		static int pushID = 0;
		for (uint16_t i = 0; i < materials.size(); ++i)
		{
			ImGui::PushID(pushID++);
			materials[i]->OnEditor();
			ImGui::PopID();
		}
		pushID = 0;
	}

	void Draw(DXDeviceContext* deviceContext)
	{
		for (uint16_t i = 0; i < subMeshCount; i++)
		{
			materials[std::min<uint16_t>(subMeshes[i].materialIndex, materials.size() - 1)]->Bind();
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
	[[nodiscard]] inline
		D3D11_TEXTURE_ADDRESS_MODE AssimpToD3D11_Wrap(const aiTextureMapMode& aimode);

	Texture* ImportTexture(aiMaterial* const& aiMaterial, aiTextureType textureType, Texture* defaultTexture);

	MeshRenderer* LoadMesh(const std::string& path);
}