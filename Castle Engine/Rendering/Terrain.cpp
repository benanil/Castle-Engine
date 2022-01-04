#include "Terrain.hpp"
// #include "../External/FastNoiseLite.hpp"
#include "Texture.hpp"

#ifndef NDEBUG
#	include "../Editor/Editor.hpp"
#endif

#include "FastNoiseLite.h"
#include "spdlog/spdlog.h"

namespace Terrain
{
	DXBuffer* vertexBuffer;
	DXBuffer*  indexBuffer;

	DXBuffer* waterVertexBuffer;
	DXBuffer* waterIndexBuffer;

	DXDeviceContext* d3d11DevCon;
	DXInputLayout* inputLayout;
	Shader* shader;

	Texture* grassTexture;
	Texture* dirtTexture;
	
	float textureScale = 0.085f;
	float noiseScale = 10.0f;
	float scale = 20.0f;
	float height = 120;

	float GetTerrainScale() { return textureScale;  };
	void BindShader() { shader->Bind(); };
	
	// forward declaration
	void Create();
}

void Terrain::Initialize()
{
	shader = new Shader("Terrain.hlsl", "Terrain.hlsl");
	grassTexture = new Texture("Textures/Grass00seamless.jpg",D3D11_TEXTURE_ADDRESS_MIRROR);
	dirtTexture = new Texture("Textures/Dirt00seamless.jpg",D3D11_TEXTURE_ADDRESS_MIRROR);
	
	inputLayout = TerrainVertex::GetLayout(shader->VS_Buffer);
	d3d11DevCon = Engine::GetDeviceContext();
	Create();
}

void Terrain::Create()
{
	TerrainVertex vertices[t_vertexCount];
	uint32_t indices[t_indexCount];

	FastNoiseLite fastNoise;
	fastNoise.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Perlin);
	fastNoise.SetFractalType(FastNoiseLite::FractalType::FractalType_FBm);

	glm::vec2 minXZ { 0.0f, 0.0f}; glm::vec2 maxXZ { t_width * scale, height * scale} ;

	for (uint32_t h = 0, i = 0; h <= t_height; h++)
	{
		for (uint32_t w = 0; w <= t_width; w++, i++)
		{
			vertices[i].pos.x = w * scale;
			vertices[i].pos.z = h * scale;
			float _noise = fastNoise.GetNoise((float)w * noiseScale, (float)h * noiseScale);
			vertices[i].height = _noise * 10;
			vertices[i].pos.y  = glm::max(_noise * height, -20.0f);
		}
	}
	
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
	
	std::vector<glm::vec3> tempNormal(t_indexCount);
	
	//Compute face normals
	for(uint32_t i = 0; i < t_indexCount / 3; ++i)
	{
		//Get the vector describing one edge of our triangle (edge 0,2)
		glm::vec3 edge1 = vertices[indices[i * 3]].pos - vertices[indices[i * 3 + 2]].pos;
		//Get the vector describing another edge of our triangle (edge 2,1)
		glm::vec3 edge2 = vertices[indices[i * 3 + 2]].pos - vertices[indices[i * 3 + 1]].pos;
		
		tempNormal[i] = glm::cross(edge2, edge1);
	}
	
	//Go through each vertex
	for(uint32_t i = 0; i < t_vertexCount; ++i)
	{
		uint32_t facesUsing = 0;
		glm::vec3 normalSum = glm::vec3(0, 0, 0);
		//Check which triangles use this vertex
		for(uint32_t j = 0; j < t_indexCount / 3; ++j)
		{
			if(indices[j * 3] == i ||
				indices[j * 3 + 1] == i ||
				indices[j * 3 + 2] == i)
			{
				normalSum += tempNormal[j];
				facesUsing++;
			}
		}
		
		vertices[i].normal = glm::normalize(normalSum / (float)facesUsing);
	}
	
	DX_RELEASE(vertexBuffer);
	DX_RELEASE(indexBuffer);

	CSCreateVertexIndexBuffers<TerrainVertex, uint32_t>(
		Engine::GetDevice(), Engine::GetDeviceContext(),
		vertices, indices, t_vertexCount, t_indexCount,
		&vertexBuffer, &indexBuffer);
}

void Terrain::Draw()
{
	dirtTexture->Bind(d3d11DevCon, 0);
	grassTexture->Bind(d3d11DevCon, 1);

	DrawIndexedInfo drawInfo {
		d3d11DevCon, inputLayout,
		vertexBuffer, indexBuffer, t_indexCount
	};
	DrawIndexed32<TerrainVertex>(&drawInfo);
}

void Terrain::OnEditor()
{
	ImGui::DragFloat("Scale", &scale, 1.0f);
	ImGui::DragFloat("Texture Scale", &textureScale, 0.001f);
	ImGui::DragFloat("Noise Scale", &noiseScale, 0.25f);
	ImGui::DragFloat("height", &height, 0.25f);

	if (ImGui::Button("Recreate"))
	{
		Create();
	}
}

void Terrain::Dispose()
{
	DX_RELEASE(indexBuffer)
	DX_RELEASE(vertexBuffer)
}
