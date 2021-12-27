#pragma once
#include "../Helper.hpp"
#include "../Engine.hpp"
#include <D3DX11tex.h>

class Texture
{
public:
	DXShaderResourceView* resourceView;
	DXTexSampler* textureSampler;
	D3D11_TEXTURE_ADDRESS_MODE  wrapMode;
	const char* wrapName;
	char* path;
public:
	Texture(const char* _path, D3D11_TEXTURE_ADDRESS_MODE _wrapMode = D3D11_TEXTURE_ADDRESS_WRAP) 
	: wrapMode(_wrapMode), path(const_cast<char*>(_path))
	{
		wrapName = WrapToString(_wrapMode);
		wchar_t* wcs = (wchar_t*)malloc(strlen(path) * 4);
		mbstowcs(wcs, path, strlen(path)+1);
		Load(wcs);
	}

	Texture(const wchar_t* _path, D3D11_TEXTURE_ADDRESS_MODE _wrapMode = D3D11_TEXTURE_ADDRESS_WRAP) 
	: wrapMode(_wrapMode)
	{
		path = (char*)malloc(wcslen(_path));
		wcstombs(path, _path, wcslen(_path) + 1);
		wrapName = WrapToString(_wrapMode);
		Load(_path);
	}

	void Bind(DXDeviceContext* d3d11DevCon, UINT slot = 0)
	{
		if (resourceView)
		{
			d3d11DevCon->PSSetShaderResources(slot, 1, &resourceView);
			d3d11DevCon->PSSetSamplers(slot, 1, &textureSampler);
		}
	}

	static void UnBind(DXDeviceContext* d3d11DevCon)
	{
		d3d11DevCon->PSSetShaderResources(0, 0, nullptr);
		d3d11DevCon->PSSetSamplers(0, 0, nullptr);
	}

	[[nodiscard]] static inline const char* WrapToString(D3D11_TEXTURE_ADDRESS_MODE mode)
	{
		switch (mode)
		{
			case D3D11_TEXTURE_ADDRESS_WRAP:        return "WrapMode_WRAP";
			case D3D11_TEXTURE_ADDRESS_CLAMP:       return "WrapMode_CLAMP";
			case D3D11_TEXTURE_ADDRESS_MIRROR_ONCE: return "WrapMode_MIRROR_ONCE";
			case D3D11_TEXTURE_ADDRESS_MIRROR:      return "WrapMode_MIRROR";
			default:	return "UNKNOWN";
		}
	}

	void SetWrapMode(D3D11_TEXTURE_ADDRESS_MODE _wrapMode)
	{
		auto device = Engine::GetDevice();
		DX_RELEASE(textureSampler);
		DX_CREATE(D3D11_SAMPLER_DESC, sampDesc);
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = _wrapMode;
		sampDesc.AddressV = _wrapMode;
		sampDesc.AddressW = _wrapMode;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

		device->CreateSamplerState(&sampDesc, &textureSampler);
	}
private:
	void Load(const wchar_t* _path)
	{
		auto device = Engine::GetDevice();

		DX_CHECK(
			D3DX11CreateShaderResourceViewFromFile(device, _path, NULL, NULL,
					&resourceView, NULL), path)

		DX_CREATE(D3D11_SAMPLER_DESC, sampDesc);
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = wrapMode;
		sampDesc.AddressV = wrapMode;
		sampDesc.AddressW = wrapMode;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		
		device->CreateSamplerState(&sampDesc, &textureSampler);
	}
};