#pragma once
#include "../Rendering.hpp"
#include "../DirectxBackend.hpp"
#include "../Engine.hpp"
#include <D3DX11tex.h>
// #include "../Structures/HString.hpp"
#include <string>
#include <exception>
#include <filesystem>
#include "../Helper.hpp"

static inline wchar_t* ToWChar(const char* string)
{
	const int len = std::strlen(string);
	wchar_t* buffer = (wchar_t*)std::malloc(len + 1 * sizeof(wchar_t));
	std::memset(buffer, 0, len + 1 * sizeof(wchar_t));
#pragma warning(suppress : 4996)
	std::mbstowcs(buffer, string, len + 1);
	return buffer;
}

class Texture
{
public:
	DXShaderResourceView* resourceView;
	DXTexSampler* textureSampler;
	D3D11_TEXTURE_ADDRESS_MODE  wrapMode;
	const char* wrapName;
	std::string path;
	unsigned int width, height;
public:
	Texture() { }
	Texture(std::string _path, D3D11_TEXTURE_ADDRESS_MODE _wrapMode = D3D11_TEXTURE_ADDRESS_WRAP) 
	: wrapMode(_wrapMode), path(_path)
	{
		wrapName = WrapToString(_wrapMode);
		std::wstring wpath = std::wstring(path.begin(), path.end());
		Load(wpath.c_str());
	}

	Texture(const wchar_t* _path, D3D11_TEXTURE_ADDRESS_MODE _wrapMode = D3D11_TEXTURE_ADDRESS_WRAP) 
	: wrapMode(_wrapMode)
	{
		path = std::string(_path, _path + std::wcslen(_path));
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
		auto device = DirectxBackend::GetDevice();
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
		using namespace std;
		if (!filesystem::exists(filesystem::path(_path)))
		{
			cout << _path << endl;
			throw std::exception("texture path is not exist!");
		}
		
		auto device = DirectxBackend::GetDevice();
		
		D3DX11_IMAGE_LOAD_INFO loadInfo;
		DX_CHECK(
			D3DX11CreateShaderResourceViewFromFile(device, _path, &loadInfo, NULL,
					&resourceView, NULL), path)

		width = loadInfo.Width;
		height = loadInfo.Height;

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