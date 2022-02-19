#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>       
#include <assimp/postprocess.h> 
#include <array>
#include <utility>
#include <vector>
#include <glm/glm.hpp>

#include <cstdint>
#include "../Rendering.hpp"
#include "../ECS/ECS.hpp"
#include "Renderer3D.hpp"
#include "Texture.hpp"

#ifndef NEDITOR
#	include "../Editor/Editor.hpp"
#endif NEDITOR

#include "AssetManager.hpp"
#include "Material.hpp"
#include "Primitives.hpp"

#include <chrono>
#include <iostream>

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
#ifndef NEDITOR
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
#endif
	void Draw(DXDeviceContext* deviceContext)
	{
		Renderer3D::SetModelMatrix(entity->transform->GetMatrix());
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

struct SphereCreateResult
{
	uint32_t vertexCount, indexCount;
	glm::vec3* vertices;
	uint32_t* indices;
};

static SphereCreateResult* CSCreateSphereVertexIndices(uint16_t LatLines, uint16_t LongLines)
{
	SphereCreateResult* result = new SphereCreateResult();
	
	result->vertexCount = ((LatLines - 2) * LongLines) + 2;
	result->indexCount  = (((LatLines - 3) * (LongLines) * 2) + (LongLines * 2)) * 3;
	
	float sphereYaw = 0.0f;
	float spherePitch = 0.0f;
	
	// calculate vertices
	result->vertices = (glm::vec3*)malloc(sizeof(glm::vec3) * result->vertexCount);
	XMVECTOR currVertPos = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	
	result->vertices[0].x = 0.0f;
	result->vertices[0].y = 0.0f;
	result->vertices[0].z = 1.0f;
	
	for (uint16_t i = 0; i < LatLines - 2; ++i)
	{
		spherePitch = (i + 1) * (DX_PI / (LatLines - 1));
		auto Rotationx = XMMatrixRotationX(spherePitch);
		for (uint16_t j = 0; j < LongLines; ++j)
		{
			sphereYaw = j * (DX_TWO_PI / (LongLines));
			auto Rotationy = XMMatrixRotationZ(sphereYaw);
			currVertPos = XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), (Rotationx * Rotationy));
			currVertPos = XMVector3Normalize(currVertPos);
			result->vertices[i * LongLines + j + 1].y = XMVectorGetY(currVertPos);
			result->vertices[i * LongLines + j + 1].x = XMVectorGetX(currVertPos);
			result->vertices[i * LongLines + j + 1].z = XMVectorGetZ(currVertPos);
		}
	}
	
	result->vertices[result->vertexCount - 1].x = 0.0f;
	result->vertices[result->vertexCount - 1].y = 0.0f;
	result->vertices[result->vertexCount - 1].z = -1.0f;
	
	// calculate indices
	result->indices = (uint32_t*)malloc(sizeof(uint32_t) * result->indexCount);
	uint32_t k = 0;
	for (uint16_t l = 0; l < LongLines - 1; ++l)
	{
		result->indices[k] = 0;
		result->indices[k + 1] = l + 1;
		result->indices[k + 2] = l + 2;
		k += 3;
	}
	
	result->indices[k] = 0;
	result->indices[k + 1] = LongLines;
	result->indices[k + 2] = 1;
	k += 3;
	
	for (uint16_t i = 0; i < LatLines - 3; ++i)
	{
		for (uint16_t j = 0; j < LongLines - 1; ++j)
		{
			result->indices[k] = i * LongLines + j + 1;
			result->indices[k + 1] = i * LongLines + j + 2;
			result->indices[k + 2] = (i + 1) * LongLines + j + 1;
			
			result->indices[k + 3] = (i + 1) * LongLines + j + 1;
			result->indices[k + 4] = i * LongLines + j + 2;
			result->indices[k + 5] = (i + 1) * LongLines + j + 2;
			
			k += 6; // next quad
		}
		
		result->indices[k] = (i * LongLines) + LongLines;
		result->indices[k + 1] = (i * LongLines) + 1;
		result->indices[k + 2] = ((i + 1) * LongLines) + LongLines;
		
		result->indices[k + 3] = ((i + 1) * LongLines) + LongLines;
		result->indices[k + 4] = (i * LongLines) + 1;
		result->indices[k + 5] = ((i + 1) * LongLines) + 1;
		
		k += 6;
	}
	
	for (uint16_t l = 0; l < LongLines - 1; ++l)
	{
		result->indices[k] = result->vertexCount - 1;
		result->indices[k + 1] = (result->vertexCount - 1) - (l + 1);
		result->indices[k + 2] = (result->vertexCount - 1) - (l + 2);
		k += 3;
	}
	
	result->indices[k]     = result->vertexCount - 1;
	result->indices[k + 1] = (result->vertexCount - 1) - LongLines;
	result->indices[k + 2] = result->vertexCount - 2;

	return result;
}

static SubMesh* CSCreateSphere(uint16_t LatLines, uint16_t LongLines)
{
	SubMesh* result = new SubMesh();
	 
	SphereCreateResult* createResult = CSCreateSphereVertexIndices(LatLines, LongLines);

	result->vertices = (Vertex*)malloc(sizeof(Vertex) * result->vertexCount);

	for (uint16_t i = 0; i < result->vertexCount; ++i)
	{
		result->vertices[i].pos = createResult->vertices[i];
	}

	result->indices = createResult->indices;
	result->vertexCount = createResult->vertexCount;
	result->indexCount  =  createResult->indexCount;

	for (uint16_t i = 0; i < result->vertexCount; ++i)
	{
		result->vertices->normal = result->vertices->pos;
	}
	
	result->CreateDXBuffers();
	return result;
}

struct PointsAndIndices32 {
	glm::vec3* vertices;
	uint32_t* indices;
	uint32_t vertexCount, indexCount;
	void Clear() {
		free(vertices); free(indices);
	}
};

static inline PointsAndIndices32 
CSCreatePlanePoints(uint32_t width, uint32_t height, glm::vec2 pos, float scale = 15.0f)
{
	uint32_t vertexCount = (width + 1) * (height + 1);
	uint32_t indexCount = width * height * 6;

	PointsAndIndices32 result  { 
		(glm::vec3*)malloc(sizeof(glm::vec3) * vertexCount), 
		(uint32_t*)malloc(sizeof(uint32_t) * indexCount) 
	};
	
	pos *= scale;
	// calculate positions
	for (uint32_t h = 0, i = 0; h < height + 1; h++)
	{
		for (uint32_t w = 0; w < width + 1; w++, i++)
		{
			result.vertices[i].x = w * scale;
			result.vertices[i].z = h * scale;
			result.vertices[i].y = 0;

			result.vertices[i].x += pos.x;
			result.vertices[i].z += pos.y;
		}
	}

	// calculate indices
	for (uint32_t ti = 0, vi = 0, y = 0; y < height; y++, vi++)
	{
		for (uint32_t x = 0; x < width; x++, ti += 6, vi++)
		{
			result.indices[ti] = vi;
			result.indices[ti + 1] = vi + width + 1;
			result.indices[ti + 2] = vi + 1;

			result.indices[ti + 3] = vi + 1;
			result.indices[ti + 4] = vi + width + 1;
			result.indices[ti + 5] = vi + width + 2;
		}
	}
	result.vertexCount = vertexCount;
	result.indexCount = indexCount;
	return result;
}

namespace MeshLoader
{
	[[nodiscard]] inline
		D3D11_TEXTURE_ADDRESS_MODE AssimpToD3D11_Wrap(const aiTextureMapMode& aimode);

	Texture* ImportTexture(aiMaterial* const& aiMaterial, aiTextureType textureType, Texture* defaultTexture);

	MeshRenderer* LoadMesh(const std::string& path);
}