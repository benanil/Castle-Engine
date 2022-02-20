#pragma once
#include <d3d11.h>
#include "GrassGroup.hpp"

namespace GrassRenderer
{
	void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	void SetShader(const XMMATRIX& view, const XMMATRIX& projection);
	void Render(const GrassGroup& grassGroup);
	void OnEditor();
	void EndRender();
}
