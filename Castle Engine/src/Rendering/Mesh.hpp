#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>       
#include <utility>
#include <vector>
#include <iostream>
#include <bitset>
#include "../Math.hpp"
#include "../Rendering.hpp"
#include "../ECS/ECS.hpp"
#include "../Transform.hpp"
#include "../CE_Common.hpp"
#include "Renderer3D.hpp"
#include "Texture.hpp"
#include "Material.hpp"
#include "Primitives.hpp"

#define MAXIMUM_MESH 1024
#define CMESH_VERSION (1)

typedef std::bitset<2048*2> CullingBitset;

struct SphereCreateResult
{
	uint32_t vertexCount, indexCount;
	glm::vec3* vertices;
	uint32_t* indices;
};

struct PointsAndIndices32 {
	glm::vec3* vertices;
	uint32_t* indices;
	uint32_t vertexCount, indexCount;
	void Clear() {
		free(vertices); free(indices);
	}
};

class Mesh
{
public:
	std::vector<SubMesh> subMeshes;
	std::vector<Material*> materials;
	std::string name;
	uint16_t subMeshCount = 0;
	Mesh() { }
	Mesh(const std::string& _name) : name(_name) { }
	void SaveMesh(const std::string& directory);
	void LoadMesh(const std::string& directory);
};

class MeshRenderer : ECS::Component
{
public:
	Mesh* mesh;
public:
	MeshRenderer();
	
	MeshRenderer(Mesh* _mesh);

	ECS::Entity* GetEntityConst() const;
	ECS::Entity* GetEntity() ;
	void SetEntity(ECS::Entity* _entity) ;
	const ECS::Transform* GetTransform() const;

	void Update(const float& deltaTime);
#ifndef NEDITOR
	void OnEditor();
#endif
	int CalculateCullingBitset(
		CullingBitset& bitset,
		uint32_t& startIndex,
		const std::array<CMath::OrthographicPlane, 4>& planes,
		const XMMATRIX& viewProjection) const;

	void Draw(DXDeviceContext* deviceContext, CullingBitset& cullData, uint32_t& startIndex);

	void RenderForShadows(DXDeviceContext* deviceContext, CullingBitset& bitset, uint32_t& startIndex);
	
	void Dispose();
};

// todo: make this constexpr
SphereCreateResult* CSCreateSphereVertexIndices(uint16_t LatLines, uint16_t LongLines);

SubMesh* CSCreateSphere(uint16_t LatLines, uint16_t LongLines);

// todo: make constexpr
PointsAndIndices32
CSCreatePlanePoints(uint32_t width, uint32_t height, glm::vec2 pos, float scale = 15.0f);

namespace MeshLoader
{
	[[nodiscard]] inline
		D3D11_TEXTURE_ADDRESS_MODE AssimpToD3D11_Wrap(const aiTextureMapMode& aimode);

	Texture* ImportTexture(aiMaterial* const& aiMaterial, aiTextureType textureType, Texture* defaultTexture);

	Mesh* LoadMesh(const std::string& path);
	
	Texture* GetWhiteTexture();
	Texture* GetBlackTexture();
	Texture* GetFlatNormalTexture();
	Texture* GetMissingTexture();

	Material* GetDefaultMaterial();
}
