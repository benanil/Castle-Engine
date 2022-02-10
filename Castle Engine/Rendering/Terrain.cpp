#include "Terrain.hpp"
#include "Texture.hpp"

#ifndef NEDITOR
#	include "../Editor/Editor.hpp"
#endif

#include "spdlog/spdlog.h"
#include "FastSIMD/FastSIMD.h"
#include "FastNoise/FastNoise.h"
#include "Renderer3D.hpp"
#include <cassert>
#include <algorithm>
#include <chrono>
#include "../Timer.hpp"
#include <omp.h>

// SMID OpenMP optimized terrain generator

namespace Terrain
{
	constexpr uint16_t t_width = 100, t_height = 100;
	constexpr uint16_t t_vertexCount = (t_width + 1) * (t_height + 1);
	constexpr uint16_t leftUpper = t_vertexCount - t_width - 1;
	constexpr uint16_t t_indexCount = t_width * t_height * 6;
	constexpr uint16_t WidthSize = t_width + 1, HeightSize = t_height + 1;

	std::vector<DXBuffer*> vertexBuffers;
	std::vector<DXBuffer*> indexBuffers;

	DXDeviceContext* d3d11DevCon;
	DXInputLayout* inputLayout;
	Shader* shader;

	Texture* grassTexture;
	Texture* dirtTexture;

	float textureScale = 0.085f;
	float noiseScale = 1.0f;
	float scale = 15.0f;
	float height = 200;
	float seaLevel = -25;
	int seed;
	bool isPerlin = false;
	glm::ivec2 CuhunkSize = { 5, 5 };

	float GetTerrainScale() { return textureScale; };
	void BindShader() { shader->Bind(); };

	void GenerateNoise(FastNoise::SmartNode<> generator, float* noise, glm::ivec2 offset);
	void CreateChunk(TerrainVertex* vertices, uint32_t* indices, std::vector<float> noise, glm::vec2 pos);
	void CalculateNormals(TerrainVertex* vertices, const uint32_t* indices);

	void Create();
}

float Terrain::GetTextureScale() { return textureScale; }

void Terrain::Initialize()
{
	shader = new Shader("Shaders/Terrain.hlsl\0");
	grassTexture = new Texture("Textures/grass_seamless.jpg", D3D11_TEXTURE_ADDRESS_MIRROR);
	dirtTexture = new Texture("Textures/dirt_texture.png", D3D11_TEXTURE_ADDRESS_MIRROR);

	inputLayout = TerrainVertex::GetLayout(shader->VS_Buffer);
	d3d11DevCon = DirectxBackend::GetDeviceContext();
	Create();
}

void Terrain::CalculateNormals(TerrainVertex* vertices, const uint32_t* indices)
{
	std::vector<XMVECTOR> tempNormal(t_indexCount); // xmvector is packed data = 16byte better than glm::vec3
	tempNormal.resize(t_indexCount);

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
	}

	{
		CSTIMER("normal calculation speed: ")
			//Go through each vertex
#pragma omp parallel for collapse(2), shared(vertices, indices) // 4x optimization
			for (int32_t i = 0; i < t_vertexCount; ++i)
			{
				uint8_t facesUsing = 0;
				XMVECTOR normalSum = XMVectorReplicate(0.0f);
				//Check which triangles use this vertex
				for (uint16_t j = 0; j < t_indexCount / 3; ++j)
				{
					if (indices[j * 3 + 0] == i ||
						indices[j * 3 + 1] == i ||
						indices[j * 3 + 2] == i)
					{
						normalSum += tempNormal[j];
						facesUsing++;
					}
				}
				// smid normalize
				vertices[i].normvec = XMVector3Normalize(XMVectorDivide(normalSum, XMVectorReplicate((float)facesUsing)));
				vertices[i].normal.y = glm::max(vertices[i].normal.y, 0.2f);
			}
	}
}

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
	CalculateNormals(vertices, indices);
}


void Terrain::GenerateNoise(FastNoise::SmartNode<> generator, float* noise, glm::ivec2 offset)
{

	// CSTIMER("Noise generation: ");
	generator->GenUniformGrid2D(noise, offset.x, offset.y, WidthSize, HeightSize, noiseScale * 0.01f, seed);
}

TerrainCreateResult Terrain::CreateSingleChunk()
{
	TerrainCreateResult result;
	result.vertices = (TerrainVertex*)malloc(sizeof(TerrainVertex) * TERRAIN_VERTEX_COUNT);
	result.indices  = (uint32_t*)malloc(sizeof(uint32_t) * TERRAIN_INDEX_COUNT);
	std::vector<float> noise;
	noise.resize(t_vertexCount);

	CreateChunk(result.vertices, result.indices, noise, glm::vec2(0, 0));
	for (uint32_t i = 0; i < TERRAIN_VERTEX_COUNT; ++i) { result.vertices[i].pos.y = 100; }

	return result;
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

	for (uint8_t x = 0, i = 0; x < CuhunkSize.y; ++x)
	{
		for (uint8_t y = 0; y < CuhunkSize.x; ++y, ++i)
		{
			GenerateNoise(fnGenerator, noise.data(), i_startPos + glm::ivec2(t_width * x, t_height * y));
			CreateChunk(vertices, indices, noise, startPos + glm::vec2(t_width * x, t_height * y));

			CSCreateVertexIndexBuffers<TerrainVertex, uint32_t>(DirectxBackend::GetDevice(), vertices, indices,
				t_vertexCount, t_indexCount, &vertexBuffers[i], &indexBuffers[i]);
		}
	}
}

void Terrain::Draw()
{
	dirtTexture->Bind(d3d11DevCon, 0);
	grassTexture->Bind(d3d11DevCon, 1);

	DrawIndexedInfo drawInfo{
		d3d11DevCon, inputLayout
	};

	drawInfo.indexCount = t_indexCount;

	// todo add frustum culling: don't draw some of the cunks
	for (uint8_t i = 0; i < vertexBuffers.size(); ++i)
	{
		drawInfo.vertexBuffer = vertexBuffers[i];
		drawInfo.indexBuffer = indexBuffers[i];
		DrawIndexed32<TerrainVertex>(&drawInfo);
	}
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

		ImGui::DragInt2("ChunkSize", &CuhunkSize.x);
		ImGui::Checkbox("Perlin?", &isPerlin);

		if (ImGui::Button("Recreate"))
		{
			Create();
		}
	}
}
#endif

void Terrain::Dispose()
{
	for (uint32_t i = 0; i < indexBuffers.size(); i++)
	{
		DX_RELEASE(indexBuffers[i]);
		DX_RELEASE(vertexBuffers[i]);
	}
}