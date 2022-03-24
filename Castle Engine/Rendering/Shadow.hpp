#pragma once
#include "RenderTexture.hpp"
#include "../Math.hpp"

namespace Shadow
{
	void Initialize();
	void UpdateShadows();
	const XMMATRIX& GetViewProjection();
	const std::array<CMath::OrthographicPlane, 4>& GetFrustumPlanes();
	/// <summary> returns aabb min max </summary>
	const glm::vec4& GetFrustumMinMax();

	void SetShadowMatrix(const XMMATRIX& model, UINT LightMatrixCBIndex);
	void Dispose();
	void BeginRenderShadowmap();
	void BindShadowTexture(UINT slot);
	void EndRenderShadowmap(D3D11_VIEWPORT* viewPort);
}