#pragma once
#include "../Helper.hpp"
#include "../Engine.hpp"

class Texture
{
public:
	Texture(const wchar_t* path)
	{
		auto device = Engine::GetDevice();
		
		DX_CHECK(
			D3DX11CreateShaderResourceViewFromFile(device, path, NULL, NULL,
			&resourceView, NULL), "texture creation failed!" 
		);

		DX_CREATE(D3D11_SAMPLER_DESC, sampDesc);
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

		device->CreateSamplerState(&sampDesc, &textureSampler);
	}

	void Bind(DXDeviceContext* d3d11DevCon)
	{
		d3d11DevCon->PSSetShaderResources(0, 1, &resourceView);
		d3d11DevCon->PSSetSamplers(0, 1, &textureSampler);
	}

	DXShaderResourceView* resourceView;
	DXTexSampler* textureSampler;
};