#pragma once
#include "../Helper.hpp"
#include "../Engine.hpp"

class Texture
{
public:
	DXShaderResourceView* resourceView;
	DXTexSampler* textureSampler;
	D3D11_TEXTURE_ADDRESS_MODE wrapMode = D3D11_TEXTURE_ADDRESS_WRAP;
public:
	Texture(const char* path, D3D11_TEXTURE_ADDRESS_MODE _wrapMode = D3D11_TEXTURE_ADDRESS_WRAP) : wrapMode(_wrapMode)
	{
		wchar_t* wcs = (wchar_t*)malloc(strlen(path) * 4);
		mbstowcs(wcs, path, strlen(path)+1);
		Load(wcs);
		free(wcs);
	}

	Texture(const wchar_t* path, D3D11_TEXTURE_ADDRESS_MODE _wrapMode = D3D11_TEXTURE_ADDRESS_WRAP) : wrapMode(_wrapMode)
	{
		Load(path);
	}

	void Bind(DXDeviceContext* d3d11DevCon)
	{
		if (resourceView)
		{
			d3d11DevCon->PSSetShaderResources(0, 1, &resourceView);
			d3d11DevCon->PSSetSamplers(0, 1, &textureSampler);
		}
	}

	static void UnBind(DXDeviceContext* d3d11DevCon)
	{
		d3d11DevCon->PSSetShaderResources(0, 0, nullptr);
		d3d11DevCon->PSSetSamplers(0, 0, nullptr);
	}

private:
	void Load(const wchar_t* path)
	{
		auto device = Engine::GetDevice();

		D3DX11CreateShaderResourceViewFromFile(device, path, NULL, NULL,
			&resourceView, NULL);

		DX_CHECK(
			D3DX11CreateShaderResourceViewFromFile(device, path, NULL, NULL,
					&resourceView, NULL), "texture importing failed"
		)

		DX_CREATE(D3D11_SAMPLER_DESC, sampDesc);
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = wrapMode;
		sampDesc.AddressV = wrapMode;
		sampDesc.AddressW = wrapMode;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		
		device->CreateSamplerState(&sampDesc, &textureSampler);
	}
};