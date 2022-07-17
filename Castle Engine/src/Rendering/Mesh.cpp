#include "Mesh.hpp"
#include "Primitives.hpp"
#include "AssetManager.hpp"
#include <fstream>
#include "../Helper.hpp"
#include <cstdint>
#include "../Timer.hpp"
#include "Shadow.hpp"
#include <iostream>

#ifndef NEDITOR
#	include "../Editor/Editor.hpp"
#endif NEDITOR

// --- MESH ---

void Mesh::SaveMesh(const std::string& directory)
{
	std::ofstream ofs = std::ofstream(directory + name + ".cmesh", std::ios::out | std::ios::binary);
	// int version = CMESH_VERSION; // write version
	// ofs.write((char*)&version, sizeof(int));
	
	int nameLen = name.size(); // write name 
	ofs.write((char*)&nameLen, sizeof(int));
	ofs.write(name.c_str(), name.size());
	
	ofs.write((char*)&subMeshCount, sizeof(uint16_t));
	for (uint16_t i = 0; i < subMeshCount; ++i) {
		subMeshes[i].SaveMesh(ofs);
	}
	// save materials
	uint16_t matSize = static_cast<uint16_t>(materials.size());
	ofs.write((char*)&matSize, sizeof(uint16_t));

	for (auto& material : materials) 
	{
		std::string materialPath = "Data\\" + name + material->name + ".mat";
		matSize = static_cast<uint16_t>(materialPath.size());
		ofs.write((char*)&matSize, sizeof(uint16_t));
		ofs.write(materialPath.c_str(), matSize);
	}
	ofs.close();
}

void Mesh::LoadMesh(const std::string& path)
{
	std::ifstream ifs = std::ifstream(path, std::ios::in | std::ios::binary);
	//SkipBOM(ifs);
	// int version = 0;
	// ifs.read((char*)&version, sizeof(int));
	// 
	// if (version != CMESH_VERSION) {
	// 	std::cout << "Mesh.cpp: cmesh version is not same as engine's cmesh version! "
	// 		<< "version: " << version << "engine version:" << CMESH_VERSION << std::endl;
	// 	throw std::exception("cmesh version is not same as engine's cmesh version!");
	// }
	
	int nameLen = 0; // read name
	ifs.read((char*)&nameLen, sizeof(int));
	name.resize(nameLen);
	ifs.read(&name[0], nameLen);

	// delete existing submeshes
	for (uint16_t i = 0; i < subMeshCount; ++i) {
		subMeshes[i].Dispose();
	}
	subMeshes.clear();

	ifs.read((char*)&subMeshCount, sizeof(uint16_t));
	subMeshes.reserve(subMeshCount);

	for (uint16_t i = 0; i < subMeshCount; ++i) {
		SubMesh submesh = SubMesh();
		submesh.LoadMesh(ifs);
		subMeshes.push_back(submesh);
	} 

	// load materials
	uint16_t matSize = 0;
	ifs.read((char*)&matSize, sizeof(uint16_t));
	uint16_t nameSize = 0;
	materials.clear();
	materials.reserve(matSize);

	for (uint16_t i = 0; i < matSize; ++i)
	{
		ifs.read((char*)&nameSize, sizeof(uint16_t));
		char* buff = (char*)calloc(nameSize + 1, 1);
		ifs.read(buff, nameSize);
		Material* material = nullptr;
		AssetManager::TryGetMaterial(material, std::string((const char*)buff));
		
		materials.push_back(material);
		free(buff);
	}

	ifs.close();
}

// --- MESH REDNERER ---

MeshRenderer::MeshRenderer() : ECS::Component() {}
MeshRenderer::MeshRenderer(Mesh* _mesh) : mesh(_mesh), ECS::Component() {}

ECS::Entity* MeshRenderer::GetEntityConst() const { return entity; }
ECS::Entity* MeshRenderer::GetEntity() { return entity; }
void MeshRenderer::SetEntity(ECS::Entity* _entity) { entity = _entity; }
const ECS::Transform* MeshRenderer::GetTransform() const { return transform; }

void MeshRenderer::Update(const float& deltaTime) {}

#ifndef NEDITOR
void MeshRenderer::OnEditor()
{
	static int pushID = 0;
	ImGui::Indent();
	for (uint16_t i = 0; i < mesh->materials.size(); ++i)
	{
		ImGui::PushID(pushID++);
		mesh->materials[i]->OnEditor();
		ImGui::PopID();
	}
	ImGui::Unindent();
	pushID = 0;
}
#endif

int MeshRenderer::CalculateCullingBitset(
	CullingBitset& bitset,
	uint32_t& startIndex,
	const std::array<CMath::OrthographicPlane, 4>& planes,
	const XMMATRIX& viewProjection) const
{
	int totalCulledMeshes = 0;
	constexpr int MinimumVertexForCulling = 64;
	XMMATRIX matrix = entity->transform->GetMatrix();
	CMath::BoundingFrustum frustum = CMath::BoundingFrustum(viewProjection);

	for (uint16_t i = 0; i < mesh->subMeshCount; ++i)
	{
		bitset[startIndex++] = CMath::CheckAABBCulled(frustum, mesh->subMeshes[i].aabb, matrix);
		bitset[startIndex + 1023ull] = CMath::CheckAABBInFrustum(mesh->subMeshes[i].aabb, planes, matrix);
		totalCulledMeshes += bitset[startIndex + 1023ull];
	}
	return totalCulledMeshes;
}

void MeshRenderer::Draw(DXDeviceContext* deviceContext, CullingBitset& cullData, uint32_t& startIndex)
{
	// LineDrawer::SetMatrix(entity->transform->GetMatrix());
	Renderer3D::SetModelMatrix(entity->transform->GetMatrix());
	// bind & update LightViewMatrix
	Shadow::SetShadowMatrix(entity->transform->GetMatrix(), 3);

	for (uint16_t i = 0; i < mesh->subMeshCount; ++i)
	{
		if (!cullData[startIndex++]) continue;
		mesh->materials[std::min<uint16_t>(mesh->subMeshes[i].materialIndex, mesh->materials.size() - 1)]->Bind();
		mesh->subMeshes[i].Draw(deviceContext);
	}
}

void MeshRenderer::RenderForShadows(DXDeviceContext* deviceContext, CullingBitset& bitset, uint32_t& startIndex)
{
	const XMMATRIX& matrix = entity->transform->GetMatrix();
	Shadow::SetShadowMatrix(matrix, 0);

	for (uint16_t i = 0; i < mesh->subMeshCount; ++i)
	{
		if (!bitset[startIndex++]) continue;
		mesh->subMeshes[i].Draw(deviceContext);
	}
}

void MeshRenderer::Dispose()
{
	for (uint16_t i = 0; i < mesh->subMeshCount; i++)
	{
		mesh->subMeshes[i].Dispose();
	}
}

// --- MESH GENERATION ---

// todo: make this constexpr
SphereCreateResult* CSCreateSphereVertexIndices(uint16_t LatLines, uint16_t LongLines)
{
	SphereCreateResult* result = new SphereCreateResult();

	result->vertexCount = ((LatLines - 2) * LongLines) + 2;
	result->indexCount = (((LatLines - 3) * (LongLines) * 2) + (LongLines * 2)) * 3;

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

	result->indices[k] = result->vertexCount - 1;
	result->indices[k + 1] = (result->vertexCount - 1) - LongLines;
	result->indices[k + 2] = result->vertexCount - 2;

	return result;
}

SubMesh* CSCreateSphere(uint16_t LatLines, uint16_t LongLines)
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
	result->indexCount = createResult->indexCount;

	for (uint16_t i = 0; i < result->vertexCount; ++i)
	{
		result->vertices->normal = result->vertices->pos;
	}

	result->CreateDXBuffers();
	return result;
}

// todo: make constexpr
PointsAndIndices32 CSCreatePlanePoints(uint32_t width, uint32_t height, glm::vec2 pos, float scale)
{
	uint32_t vertexCount = (width + 1) * (height + 1);
	uint32_t indexCount = width * height * 6;

	PointsAndIndices32 result{
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
