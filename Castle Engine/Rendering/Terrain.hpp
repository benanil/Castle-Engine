#include "../Helper.hpp"
#include "../Engine.hpp"
#include "Shader.hpp"

struct __declspec(align(32)) TerrainVertex
{
	glm::vec3 pos;
	float height;
	glm::vec3 normal;
	
	static DXInputLayout* GetLayout(DXBlob* VS_Buffer)
	{
		DXInputLayout* vertLayout;
		
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 , D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "HEIGHT"  , 0, DXGI_FORMAT_R32_FLOAT      , 0, 12 , D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL"  , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 16 , D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		
		auto device = Engine::GetDevice();
		device->CreateInputLayout(layout, 3, VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), &vertLayout);
		return vertLayout;
	}
};

namespace Terrain
{
	constexpr uint16_t t_width = 100;
	constexpr uint16_t t_height = 100;
	constexpr uint16_t t_vertexCount = (t_width + 1) * (t_height + 1);
	constexpr uint16_t t_indexCount   = t_width * t_height * 6;

	void Initialize();

	void Draw();
	void Dispose();
	void OnEditor();

	float GetTerrainScale();
	void BindShader();
}