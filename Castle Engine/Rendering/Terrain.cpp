// SMID OpenMP optimized terrain generator

#include "Terrain.hpp"
#include "Texture.hpp"

#ifndef NEDITOR
#	include "../Editor/Editor.hpp"
#endif

#include <cassert>
#include <algorithm>
#include <chrono>

#include "spdlog/spdlog.h"
#include "FastSIMD/FastSIMD.h"
#include "FastNoise/FastNoise.h"
#include "Renderer3D.hpp"
#include "ComputeShader.hpp"
#include "GrassGroup.hpp"
#include "Grassrenderer.hpp"
#include "LineDrawer.hpp"
#include "../ECS/ECS.hpp"
#include "../Timer.hpp"
#include <limits>
#include <omp.h>

using namespace CS; // for compute shader
using namespace CMath;

namespace Terrain
{
	constexpr uint16_t t_width = 100, t_height = 100;
	constexpr uint16_t t_vertexCount = (t_width + 1) * (t_height + 1);
	constexpr uint16_t t_indexCount = t_width * t_height * 6;
	constexpr uint16_t WidthSize = t_width + 1, HeightSize = t_height + 1;
	constexpr uint32_t GrassPerChunk = TERRAIN_GRASS_PER_CHUNK ;
	// constexpr uint16_t GrassPerChunk = (t_indexCount / 3) * 2;

	std::vector<DXBuffer*> vertexBuffers;
	std::vector<DXBuffer*> indexBuffers;

	DXDeviceContext* d3d11DevCon;
	DXInputLayout* inputLayout;
	Shader* shader;

	Texture* grassTexture;
	Texture* dirtTexture;

	ComputeShader* computeShader;
	StructuredBufferHandle grassIndicesHandle, grassVertexHandle;
	RWBufferHandle grassResultHandle;
	RWBufferHandle culledGrassesHandle;

	float textureScale = 0.5;
	float noiseScale = 0.75;
	float scale = 3.4f;
	float height = 30;
	float seaLevel = -25;
	int seed;
	bool isPerlin = false;
	glm::ivec2 CuhunkSize = { 6, 6 };
	int culledTerrainCount;

	std::vector<GrassGroup*> grassGroups;
	std::vector<AABB> AABBs;

	float GetTerrainScale() { return textureScale; };
	void BindShader() { shader->Bind(); };

	void GenerateNoise(FastNoise::SmartNode<> generator, float* noise, glm::ivec2 offset);
	void CreateChunk(TerrainVertex* vertices, uint32_t* indices, std::vector<float> noise, glm::vec2 pos);
	void CalculateNormals(TerrainVertex* vertices, const uint32_t* indices, XMVECTOR* tempNormal);
	void CalculateAABB(const TerrainVertex* vertices);
	void CalculateGrassPoints(const TerrainVertex* vertices, const uint32_t* indices);

	void Create();
}

float Terrain::GetTextureScale() { return textureScale; }

void Terrain::Initialize()
{
	shader = new Shader("Shaders/Terrain.hlsl\0");
	grassTexture = new Texture("Textures/grass_seamless.jpg", D3D11_TEXTURE_ADDRESS_MIRROR);
	dirtTexture = new Texture("Textures/dirt_texture.png", D3D11_TEXTURE_ADDRESS_MIRROR);
	computeShader = new ComputeShader("Shaders/RandMeshPoints.hlsl", "CS", 64, 1);
	
	grassVertexHandle  = computeShader->CreateStructuredBuffer(sizeof(TerrainVertex), t_vertexCount, nullptr, D3D11_MAP_WRITE);
	grassIndicesHandle = computeShader->CreateStructuredBuffer(sizeof(uint32_t), t_indexCount, nullptr, D3D11_MAP_WRITE);
	grassResultHandle  = computeShader->RWCreateUAVBuffer(sizeof(XMMATRIX), GrassPerChunk, nullptr);
	culledGrassesHandle = computeShader->RWCreateUAVBuffer(sizeof(int), TERRAIN_TRIANGLE_COUNT, nullptr);

	inputLayout = TerrainVertex::GetLayout(shader->VS_Buffer);
	d3d11DevCon = DirectxBackend::GetDeviceContext();
	Create();
}

void Terrain::CalculateNormals(TerrainVertex* vertices, const uint32_t* indices, XMVECTOR* tempNormal)
{
	//Compute face normals
	for (uint32_t i = 0; i < t_indexCount / 3; ++i)
	{
		//Get the vector describing one edge of our triangle (edge 0,2)
		XMVECTOR edge1 = vertices[indices[i * 3]].posvec - vertices[indices[i * 3 + 2]].posvec;
		//Get the vector describing another edge of our triangle (edge 2,1)
		XMVECTOR edge2 = vertices[indices[i * 3 + 2]].posvec - vertices[indices[i * 3 + 1]].posvec;
		tempNormal[i] = XMVector3Cross(edge2, edge1); // smid cross
	}
	//Go through each vertex
#pragma omp parallel for collapse(2), shared(vertices, indices) // 4x optimization
	for (int32_t i = 0; i < t_vertexCount; ++i)
	{
		XMVECTOR normalSum = XMVectorReplicate(0.0f);
		//Check which triangles use this vertex
		for (uint16_t j = 0; j < t_indexCount / 3; ++j)
		{
			if(_mm_movemask_epi8(_mm_cmpeq_epi32(_mm_set1_epi32(i), _mm_loadu_epi32(indices + (j * 3))))) {
				normalSum += tempNormal[j];
			}
		}
		// smid normalize
		vertices[i].normvec = XMVector3Normalize(normalSum);
		vertices[i].normal.y = glm::max(vertices[i].normal.y, 0.2f);
	}
}
void Terrain::CalculateGrassPoints(const TerrainVertex* vertices, const uint32_t* indices)
{
	auto vertexMapResult = computeShader->MapStructuredBuffer(grassVertexHandle);
	memcpy(vertexMapResult.data, vertices, sizeof(TerrainVertex) * t_vertexCount);
	computeShader->UnmapBuffer(vertexMapResult.OutputBuffer);

	auto indexMapResult = computeShader->MapStructuredBuffer(grassIndicesHandle);
	memcpy(indexMapResult.data, indices, sizeof(uint32_t) * t_indexCount);
	computeShader->UnmapBuffer(indexMapResult.OutputBuffer);
	
	computeShader->Dispatch(TERRAIN_TRIANGLE_COUNT / 64, 1);
	
	auto computeResult = computeShader->RWMapUAVBuffer(grassResultHandle, D3D11_MAP_READ);
	auto cullResult = computeShader->RWMapUAVBuffer(culledGrassesHandle, D3D11_MAP_READ);
	
	grassGroups.push_back(new GrassGroup((XMMATRIX*)computeResult.data, (int*)cullResult.data, DirectxBackend::GetDevice()));
	
	computeShader->UnmapBuffer(computeResult.OutputBuffer);
	computeShader->UnmapBuffer(cullResult.OutputBuffer);

	shader->Bind();
}

// if (indices[j * 3 + 0] == i || indices[j * 3 + 1] == i || indices[j * 3 + 2] == i)

void Terrain::CreateChunk(
	TerrainVertex* vertices, uint32_t* indices,
	std::vector<float> noise, glm::vec2 pos)
{
	pos *= scale;
	// calculate positions
	for (uint32_t h = 0, i = 0; h < t_height + 1; h++)
	{
		for (uint32_t w = 0; w < t_width + 1; w++, i++)
		{
			float _noise = noise[h * (t_height + 1) + w];
			vertices[i].pos.x = w * scale;
			vertices[i].pos.z = h * scale;
			vertices[i].pos.y = glm::max(_noise * height, seaLevel);

			vertices[i].pos.x += pos.x;
			vertices[i].pos.z += pos.y;
		}
	}

	// calculate indices
	for (uint32_t ti = 0, vi = 0, y = 0; y < t_height; y++, vi++)
	{
		for (uint32_t x = 0; x < t_width; x++, ti += 6, vi++)
		{
			indices[ti] = vi;
			indices[ti + 1] = vi + t_width + 1;
			indices[ti + 2] = vi + 1;

			indices[ti + 3] = vi + 1;
			indices[ti + 4] = vi + t_width + 1;
			indices[ti + 5] = vi + t_width + 2;
		}
	}
}

void Terrain::CalculateAABB(const TerrainVertex* vertices)
{
	AABB aabb;
	aabb.min.x = vertices[0].pos.x;
	aabb.min.y = 20;
	aabb.min.z = vertices[0].pos.z;

	aabb.max.x = aabb.min.x + (t_width * scale);
	aabb.max.y = -20;
	aabb.max.z = aabb.min.z + (t_width * scale);
	AABBs.push_back(aabb);
}

void Terrain::GenerateNoise(FastNoise::SmartNode<> generator, float* noise, glm::ivec2 offset)
{
	// CSTIMER("Noise generation: ");
	generator->GenUniformGrid2D(noise, offset.x, offset.y, WidthSize, HeightSize, noiseScale * 0.01f, seed);
}

void Terrain::Create()
{
	Dispose();

	TerrainVertex vertices[t_vertexCount];
	uint32_t indices[t_indexCount];

	// create first terrain's noise
	std::vector<float> noise;
	noise.resize(t_vertexCount);

	FastNoise::SmartNode<> fnGenerator;

	if (isPerlin)
		fnGenerator = FastNoise::New<FastNoise::Perlin>();
	else fnGenerator = FastNoise::NewFromEncodedNodeTree("DQAFAAAAAAAAQAgAAAAAAD8AAAAAAA==");

	vertexBuffers.resize(CuhunkSize.x * CuhunkSize.y);
	indexBuffers.resize(CuhunkSize.x * CuhunkSize.y);

	glm::vec2 startPos = { -((CuhunkSize.x * t_width) / 2),  -((CuhunkSize.y * t_height) / 2) };
	glm::ivec2 i_startPos = { (int)startPos.x,  (int)startPos.y };

	XMVECTOR* tempNormal = (XMVECTOR*)malloc(sizeof(XMVECTOR) * t_indexCount);

	for (uint8_t x = 0, i = 0; x < CuhunkSize.y; ++x)
	{
		for (uint8_t y = 0; y < CuhunkSize.x; ++y, ++i)
		{
			GenerateNoise(fnGenerator, noise.data(), i_startPos + glm::ivec2(t_width * x, t_height * y));
			CreateChunk(vertices, indices, noise, startPos + glm::vec2(t_width * x, t_height * y));
			CalculateNormals(vertices, indices, tempNormal);
			CalculateGrassPoints(vertices, indices);
			CalculateAABB(vertices);

			CSCreateVertexIndexBuffers(DirectxBackend::GetDevice(), vertices, indices,
				t_vertexCount, t_indexCount, &vertexBuffers[i], &indexBuffers[i]);
		}
	}

	free(tempNormal); tempNormal = nullptr;
}

// FrustumBitset std::bitset<128> is 16 byte
void Terrain::DrawGrasses(const FreeCamera& camera, const FrustumBitset& frustumSet)
{
	const glm::vec3& cameraPos = camera.transform.GetPosition();
	glm::vec3 camForward = glm::cross(camera.transform.GetRight(), glm::vec3(0.0f, 1.0f, 0.0f));

	for (int i = 0; i < grassGroups.size(); ++i)
	{
		if (!frustumSet[i]) continue;
		GrassRenderer::Render(*grassGroups[i]);
	}
}

FrustumBitset Terrain::Draw(const FreeCamera& camera)
{
	dirtTexture->Bind(d3d11DevCon, 0);
	grassTexture->Bind(d3d11DevCon, 1);

	const glm::vec3& cameraPos = camera.transform.GetPosition();
	glm::vec3 camForward = glm::cross(camera.transform.GetRight(), glm::vec3(0.0f, 1.0f, 0.0f));
	FrustumBitset frustumSet;

	// todo add frustum culling: done!
	for (uint8_t i = 0; i < vertexBuffers.size(); ++i)
	{
		frustumSet.set(i, isTerrainCulled(AABBs[i], cameraPos, camForward, camera.fov));
		if (!frustumSet[i]) continue;
		culledTerrainCount++;

		d3d11DevCon->IASetInputLayout(inputLayout);
		d3d11DevCon->IASetIndexBuffer(indexBuffers[i], DXGI_FORMAT_R32_UINT, 0);

		UINT stride = sizeof(TerrainVertex), offset = 0;
		d3d11DevCon->IASetVertexBuffers(0, 1, &vertexBuffers[i], &stride, &offset);
		d3d11DevCon->DrawIndexed(t_indexCount, 0, 0);
	}
	return frustumSet;
}

#ifndef NEDITOR
void Terrain::OnEditor()
{
	if (ImGui::CollapsingHeader("Terrain")) 
	{
		ImGui::DragFloat("Scale", &scale, 1.0f);
		ImGui::DragFloat("Texture Scale", &textureScale, 0.001f);
		ImGui::DragFloat("Noise Scale", &noiseScale, 0.25f);
		ImGui::DragFloat("height", &height, 0.25f);
		ImGui::DragInt("Seed", &seed);
		ImGui::DragFloat("seaLevel", &seaLevel);

		ImGui::DragInt2("ChunkSize", &CuhunkSize.x, 1.0f, 0, 11); // 11 because this is maximum value that frustumbitset can handle for now sqrt(128)
		ImGui::Checkbox("Perlin?", &isPerlin);
		ImGui::TextColored({ .7,.4,0.0f, 1.0f }, std::to_string(culledTerrainCount).c_str());
		culledTerrainCount = 0;
		if (ImGui::Button("Recreate"))
		{
			Create();
		}
	}
}
#endif

void Terrain::Dispose()
{
	for (uint32_t i = 0; i < grassGroups.size(); ++i) {
		grassGroups[i]->~GrassGroup();
		grassGroups[i] = nullptr;
	}
	grassGroups.clear();
	AABBs.clear();
	for (uint32_t i = 0; i < indexBuffers.size(); i++)
	{
		DX_RELEASE(indexBuffers[i]);
		DX_RELEASE(vertexBuffers[i]);
	}
}