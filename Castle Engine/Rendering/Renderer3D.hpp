#pragma once
#include "RenderTexture.hpp"

struct InputLayoutCreateInfo
{
	const char* name; DXGI_FORMAT format;
};

namespace Renderer3D
{
	void Initialize(DXDevice* device, DXDeviceContext* deviceContext, unsigned int msaaSamples);
	void PostProcessing(DXShaderResourceView* srv, DXTexSampler* sampler, bool build);
	
	DXInputLayout* CreateVertexInputLayout(std::vector<InputLayoutCreateInfo> infos, DXBlob* VS_Buffer);
	RenderTexture* GetPostRenderTexture();

	void Dispose();
}
