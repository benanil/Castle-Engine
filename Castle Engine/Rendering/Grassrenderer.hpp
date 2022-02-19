#pragma once
#include <d3d11.h>
#include "GrassGroup.hpp"

namespace GrassRenderer
{
	void SetShader();
	void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	void Render(const GrassGroup& grassGroup);
}
