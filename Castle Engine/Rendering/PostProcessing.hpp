#pragma once
#include "Renderer3D.hpp"
#include "Shader.hpp"
#include <array>

namespace PostProcessing
{
	void Initialize(ID3D11Device* Device, ID3D11DeviceContext* DeviceContext, unsigned int _msaaSamples);
	void Proceed(DXShaderResourceView* srv, DXTexSampler* sampler, bool build);
	void WindowScaleEvent(const int& _width, const int& _height);
	Shader* GetShader();
	RenderTexture* GetPostRenderTexture() ;

	void OnEditor();
	void Dispose();
}
	
	