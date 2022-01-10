#include "Terrain.hpp"
#include "Texture.hpp"

#ifndef NEDITOR
#	include "../Editor/Editor.hpp"
#endif

#include "spdlog/spdlog.h"
#include "FastSIMD/FastSIMD.h"
#include "FastNoise/FastNoise.h"
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
	int VerticalChunkCount = 5, HorizontalChunkCount = 5;

	float GetTerrainScale() { return textureScale;  };
	void BindShader() { shader->Bind(); };
	
	std::array<uint32_t, WidthSize > XMinusIndices; 
	std::array<uint32_t, WidthSize > XPlusIndices ;
	std::array<uint32_t, HeightSize> ZMinusIndices; 
	std::array<uint32_t, HeightSize> ZPlusIndices ;
	
	const uint32_t* GetEdgeIndices(TerrainEdge edgeFlags);
	void GenerateNoise(FastNoise::SmartNode<> generator, float* noise, glm::ivec2 offset);
	void CreateChunk(TerrainVertex* vertices, uint32_t* indices, std::vector<float> noise,  glm::vec2 pos);
	void CalculateNormals(TerrainVertex* vertices, const uint32_t* indices);

	void Create();
}

float Terrain::GetTextureScale() { return textureScale; }

void Terrain::Initialize()
{
	for (uint32_t i = 0; i < t_width + 1; i++) XMinusIndices[i] = i;
	for (uint32_t i = leftUpper, j = 0; i < t_vertexCount; i++, j++) XPlusIndices[j] = i;   
	
	for (uint32_t i = 0       , j = 0; j < ZMinusIndices.size(); i += t_width  + 1 , j++) ZMinusIndices[j] = i;
	for (uint32_t i = t_height, j = 0; j < ZPlusIndices.size() ; i += t_height + 1 , j++) ZPlusIndices [j] = i;
	
	shader = new Shader("Terrain.hlsl", "Terrain.hlsl");
	grassTexture = new Texture("Textures/Grass00seamless.jpg", D3D11_TEXTURE_ADDRESS_MIRROR);
	dirtTexture = new Texture("Textures/Dirt00seamless.jpg", D3D11_TEXTURE_ADDRESS_MIRROR);
	
	inputLayout = TerrainVertex::GetLayout(shader->VS_Buffer);
	d3d11DevCon = Engine::GetDeviceContext();
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

const uint32_t* Terrain::GetEdgeIndices(TerrainEdge edgeFlags) 
{
	switch (edgeFlags)
	{
		case TerrainEdge::XMinus: return XMinusIndices.data();
		case TerrainEdge::XPlus : return XPlusIndices .data();
		case TerrainEdge::ZMinus: return ZMinusIndices.data();
		case TerrainEdge::ZPlus : return ZPlusIndices .data();
		default: assert(1, "terrain edge flag is wrong!"); return nullptr;
	}
}

void Terrain::GenerateNoise(FastNoise::SmartNode<> generator, float* noise, glm::ivec2 offset)
{
	// CSTIMER("Noise generation: ");
	generator->GenUniformGrid2D(noise, offset.x, offset.y, WidthSize, HeightSize, noiseScale * 0.01f, seed);
}

void Terrain::Create()
{
#define DX_CREATE_VERTEX_INDEX_BUFFERS(vertBuff, indBuff) CSCreateVertexIndexBuffers<TerrainVertex, uint32_t>(Engine::GetDevice(), Engine::GetDeviceContext(), vertices, indices, t_vertexCount, t_indexCount, vertBuff, indBuff);
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
	
	vertexBuffers.resize(VerticalChunkCount * HorizontalChunkCount);
	indexBuffers.resize (VerticalChunkCount * HorizontalChunkCount);
	
	glm::vec2 startPos    = { -((VerticalChunkCount * t_width) / 2),  -((HorizontalChunkCount * t_height) / 2) };
	glm::ivec2 i_startPos = { (int)startPos.x,  (int)startPos.y };

	for (uint8_t x = 0, i = 0; x < HorizontalChunkCount; ++x)
	{
		for (uint8_t y = 0; y < VerticalChunkCount; ++y, ++i)
		{
			GenerateNoise(fnGenerator, noise.data(), i_startPos + glm::ivec2(t_width * x, t_height * y));
			CreateChunk(vertices, indices, noise, startPos + glm::vec2(t_width * x, t_height * y));
			DX_CREATE_VERTEX_INDEX_BUFFERS(&vertexBuffers[i], &indexBuffers[i]);
		}
	}
}

void Terrain::Draw()
{
	dirtTexture->Bind(d3d11DevCon, 0);
	grassTexture->Bind(d3d11DevCon, 1);
	
	DrawIndexedInfo drawInfo {
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
	ImGui::DragFloat("Scale", &scale, 1.0f);
	ImGui::DragFloat("Texture Scale", &textureScale, 0.001f);
	ImGui::DragFloat("Noise Scale", &noiseScale, 0.25f);
	ImGui::DragFloat("height", &height, 0.25f);
	ImGui::DragInt("Seed", &seed);
	ImGui::DragFloat("seaLevel", &seaLevel);
	
	ImGui::DragInt("VerticalChunkCount", &VerticalChunkCount); 
	ImGui::SameLine();
	ImGui::DragInt("HorizontalChunkCount", &HorizontalChunkCount);
	ImGui::Checkbox("Perlin?", &isPerlin);

	if (ImGui::Button("Recreate"))
	{
		Create();
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
