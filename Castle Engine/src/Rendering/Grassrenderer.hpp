#pragma once
#include <d3d11.h>
#include "GrassGroup.hpp"
#include "../Math.hpp"

namespace GrassRenderer
{
	void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	void SetShader(const XMMATRIX& viewProjection);
	void Render(const GrassGroup& grassGroup);
	void OnEditor();
	void EndRender();
}
