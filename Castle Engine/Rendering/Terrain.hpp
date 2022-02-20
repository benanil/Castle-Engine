#pragma once
#include "../Rendering.hpp"
#include "../DirectxBackend.hpp"
#include "Shader.hpp"

#define TERRAIN_VERTEX_COUNT ((100 + 1) * (100 + 1))
#define TERRAIN_INDEX_COUNT   (100 * 100 * 6)
#define TERRAIN_TRIANGLE_COUNT (TERRAIN_INDEX_COUNT / 3)
#define TERRAIN_GRASS_PER_CHUNK (TERRAIN_TRIANGLE_COUNT * 3)
// #define TERRAIN_GRASS_PER_CHUNK ((TERRAIN_INDEX_COUNT / 3) * 2)

struct GrassVertex {
	glm::vec3 position;
	XMHALF2 uv;
};
__declspec(align(32)) struct TerrainVertex
{
	union {
		glm::vec3 pos;
		float padding;
		struct { XMVECTOR posvec; };
	};

	union {
		glm::vec3 normal;
		float padding1;
		struct { XMVECTOR normvec; };
	};

	static DXInputLayout* GetLayout(DXBlob* VS_Buffer)
	{
		DXInputLayout* vertLayout;

		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 , D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL"  , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 16 , D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		auto device = DirectxBackend::GetDevice();
		device->CreateInputLayout(layout, 2, VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), &vertLayout);
		return vertLayout;
	}
};

namespace Terrain
{
	void Initialize();

	void Draw();
	void DrawGrasses();
	void Dispose();
	void OnEditor();

	float GetTextureScale();
	void BindShader();
}
