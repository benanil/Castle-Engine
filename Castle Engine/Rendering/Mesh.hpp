#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>       
#include <assimp/postprocess.h> 
#include <array>
#include <utility>
#include <vector>
#include <cstdint>
#include <chrono>
#include <iostream>
#include <bitset>
#include "../Math.hpp"
#include "../Rendering.hpp"
#include "../ECS/ECS.hpp"
#include "../Timer.hpp"
#include "../Transform.hpp"
#include "Renderer3D.hpp"
#include "Texture.hpp"
#include "Shadow.hpp"

#ifndef NEDITOR
#	include "../Editor/Editor.hpp"
#endif NEDITOR

#include "AssetManager.hpp"
#include "Material.hpp"
#include "Primitives.hpp"

#define MAXIMUM_MESH 1024

typedef std::bitset<2048> CullingBitset;

struct DrawResult {
	int CulledMeshCount = 0;
	double milisecond = 0;
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
	const ECS::Transform* GetTransform() const { return transform; };

	void Update(const float& deltaTime) {}
#ifndef NEDITOR
	void OnEditor()
	{
		static int pushID = 0;
		ImGui::Indent();
		for (uint16_t i = 0; i < materials.size(); ++i)
		{
			ImGui::PushID(pushID++);
			materials[i]->OnEditor();
			ImGui::PopID();
		}
		ImGui::Unindent();
		pushID = 0;
	}
#endif
	
	int CalculateCullingBitset(
		CullingBitset& bitset,
		uint32_t& startIndex, 
		const std::array<CMath::OrthographicPlane, 4>& planes,
		const XMMATRIX& viewProjection) const
	{
		int totalCulledMeshes = 0;
		constexpr int MinimumVertexForCulling = 64;
		XMMATRIX matrix = entity->transform->GetMatrix();

		for (uint16_t i = 0; i < subMeshCount; ++i)
		{
			bitset[startIndex++] = !CMath::CheckAABBCulled(CMath::BoundingFrustum(viewProjection), subMeshes[i].aabb, matrix);
			bitset[startIndex + 1023] = CMath::CheckAABBInFrustum(subMeshes[i].aabb, planes, matrix);
			totalCulledMeshes += bitset[startIndex-1];
		}			
		return totalCulledMeshes;
	}

	void Draw(DXDeviceContext* deviceContext, CullingBitset& cullData, uint32_t& startIndex)
	{
		// LineDrawer::SetMatrix(entity->transform->GetMatrix());
		Renderer3D::SetModelMatrix(entity->transform->GetMatrix());
		// bind & update LightViewMatrix
		Shadow::SetShadowMatrix(entity->transform->GetMatrix(), 3);  

		for (uint16_t i = 0; i < subMeshCount; ++i)
		{
			if (cullData[startIndex++]) continue;
			materials[std::min<uint16_t>(subMeshes[i].materialIndex, materials.size() - 1)]->Bind();
			subMeshes[i].Draw(deviceContext);
		}
	}

	int RenderForShadows(DXDeviceContext* deviceContext, CullingBitset& bitset, uint32_t& startIndex)
	{
		int culledCount = 0;
		const XMMATRIX& matrix = entity->transform->GetMatrix();
		Shadow::SetShadowMatrix(matrix, 0);
		
		for (uint16_t i = 0; i < subMeshCount; ++i) 
		{
			if (!bitset[startIndex++]) continue;
			culledCount++;
			subMeshes[i].Draw(deviceContext);
		}
		return culledCount;
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

// todo: make this constexpr
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

static __forceinline void CSSet3Uint(uint32_t* ptr, const std::array<int, 3>& data)
{
	ptr[0] = data[0];
	ptr[1] = data[1];
	ptr[2] = data[2];
}

static inline
PointsAndIndices32 CSCreateCube(float scale)
{
	PointsAndIndices32 result {
		(glm::vec3*)malloc(sizeof(glm::vec3) * 8),
		(uint32_t*)malloc(sizeof(uint32_t) * 36),
		8,36
	};

	const float halfScale = scale * 0.5f;

	result.vertices[0] = glm::vec3(-halfScale,  halfScale,  halfScale);
	result.vertices[1] = glm::vec3( halfScale,  halfScale,  halfScale);
	result.vertices[2] = glm::vec3(-halfScale, -halfScale,  halfScale);
	result.vertices[3] = glm::vec3( halfScale, -halfScale,  halfScale);
	result.vertices[4] = glm::vec3(-halfScale,  halfScale, -halfScale);
	result.vertices[5] = glm::vec3( halfScale,  halfScale, -halfScale);
	result.vertices[6] = glm::vec3(-halfScale, -halfScale, -halfScale);
	result.vertices[7] = glm::vec3( halfScale, -halfScale, -halfScale);

	CSSet3Uint(result.indices + 0 , { 0, 1, 2 });
	CSSet3Uint(result.indices + 3 , { 2, 1, 3 });
	CSSet3Uint(result.indices + 6 , { 4, 0, 6 });
	CSSet3Uint(result.indices + 9 , { 6, 0, 2 });
	CSSet3Uint(result.indices + 12, { 7, 5, 6 });
	CSSet3Uint(result.indices + 15, { 6, 5, 4 });
	CSSet3Uint(result.indices + 18, { 3, 1, 7 });
	CSSet3Uint(result.indices + 21, { 7, 1, 5 });
	CSSet3Uint(result.indices + 24, { 4, 5, 0 });
	CSSet3Uint(result.indices + 27, { 0, 5, 1 });
	CSSet3Uint(result.indices + 30, { 3, 7, 2 });
	CSSet3Uint(result.indices + 33, { 2, 7, 6 });

	return result;
}

// todo: make constexpr
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