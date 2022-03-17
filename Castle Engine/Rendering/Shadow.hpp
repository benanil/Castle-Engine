#pragma once
#include "RenderTexture.hpp"
#include "../Math.hpp"

namespace Shadow
{
	void Initialize();
	void UpdateShadows();
	const XMMATRIX& GetViewProjection();
	void SetShadowMatrix(const XMMATRIX& model, UINT LightMatrixCBIndex);
	void Dispose();
	void BeginRenderShadowmap();
	void BindShadowTexture(UINT slot);
	void EndRenderShadowmap(D3D11_VIEWPORT* viewPort);
}