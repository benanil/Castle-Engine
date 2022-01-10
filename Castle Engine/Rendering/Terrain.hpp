#include "../Helper.hpp"
#include "../Engine.hpp"
#include "Shader.hpp"

namespace Terrain
{
	struct __declspec(align(32)) TerrainVertex
	{
		union
		{
			glm::vec3 pos;
			float padding;
			struct { XMVECTOR posvec; };
		};

		union
		{
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
			
			auto device = Engine::GetDevice();
			device->CreateInputLayout(layout, 2, VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), &vertLayout);
			return vertLayout;
		}
	};
	
	enum class TerrainEdge : uint16_t 
	{
		NONE   = 0x00000000,
		XMinus = 0x00000001, // <- 
		XPlus  = 0x00000002, // ->
		ZMinus = 0x00000004, // up
		ZPlus  = 0x00000008  // down
	};
	
	void Initialize();

	void Draw();
	void Dispose();
	void OnEditor();

	float GetTextureScale();
	void BindShader();
}
