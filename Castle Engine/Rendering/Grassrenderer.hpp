#pragma once
#include <d3d11.h>

namespace GrassRenderer
{
	void SetShader();
	void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	void Render(ID3D11Buffer* instancingBuffer);
}
