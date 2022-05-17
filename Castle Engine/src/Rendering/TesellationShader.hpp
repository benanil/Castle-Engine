#pragma once
#include "../Rendering.hpp"
#include "../Engine.hpp"
#include "../Helper.hpp"
#include <cassert>
#include <D3DX11.h>
#include "../DirectxBackend.hpp"
#include <iostream>
#include <d3dcompiler.h>
#ifdef _MSC_VER
#pragma comment(lib, "d3dcompiler") // Automatically link with d3dcompiler.lib as we are using D3DCompile() below.
#endif

class TesellationShader
{
	struct Profiles
	{
		const char* vertex, *frag, *ds, *hs;
	};

public:
	DXVertexShader* VS;
	DXPixelShader* PS;
	ID3D11HullShader* HS;
	ID3D11DomainShader*DS;

	DXBlob* VS_Buffer;
	DXDevice* device;

public:

	TesellationShader() {};
	TesellationShader(const char* path) {
		Load(path);
	}

	void Bind() {
		//Set Vertex and Pixel Shaders
		d3d11DevCon->VSSetShader(VS, 0, 0); d3d11DevCon->PSSetShader(PS, 0, 0);
		d3d11DevCon->DSSetShader(DS, 0, 0); d3d11DevCon->HSSetShader(HS, 0, 0);
	}

	void Dispose()
	{
		DX_RELEASE(VS);        DX_RELEASE(PS);
		DX_RELEASE(VS_Buffer); DX_RELEASE(HS); DX_RELEASE(DS);
	}

	~TesellationShader() {
		Dispose();
	}

private:

	DXDeviceContext* d3d11DevCon;

	void Load(const char* path)
	{
		Profiles profiles = GetLatestProfiles();

		device = DirectxBackend::GetDevice();
		d3d11DevCon = DirectxBackend::GetDeviceContext();

		std::string shaderText = ReadAllText(std::string(path));
		DXBlob* errorBlob;
		
		DXBlob* PS_Buffer, *DS_Buffer, *HS_Buffer;

		D3D_SHADER_MACRO shaderMacros{};
#ifdef  DEBUG
		shaderMacros.Definition = "DEBUG";
		shaderMacros.Name = "DEBUG";
#endif //  DEBUG
		HRESULT hr = 0;
		// Create vertex shader
		{
			if (FAILED(D3DCompile(shaderText.c_str(), shaderText.size(), nullptr,
				&shaderMacros, nullptr, "VS", profiles.vertex, 0, 0, &VS_Buffer, &errorBlob))) {
				std::cout << "Vertex Shader Compiling Error:\n" << ((char*)errorBlob->GetBufferPointer()) << std::endl;
				DX_CHECK(-1, "Vertex Shader Compiling Error")
			}
			if (device->CreateVertexShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), NULL, &VS) != S_OK) {
				std::cout << "hresult: " << hr << std::endl; DX_CHECK(-1, "vertex Shader Compiling Error ")
			}
			DX_RELEASE(errorBlob);
		}
		// Create frag shader
		{
			if (FAILED(D3DCompile(shaderText.c_str(), shaderText.size(), nullptr,
				&shaderMacros, nullptr, "PS", profiles.frag, 0, 0, &PS_Buffer, &errorBlob))) {
				std::cout << ((char*)errorBlob->GetBufferPointer()) << std::endl;
				DX_CHECK(-1, "pixel Shader Compiling Error")
			}
			if (device->CreatePixelShader(PS_Buffer->GetBufferPointer(), PS_Buffer->GetBufferSize(), NULL, &PS) != S_OK) {
				std::cout << "hresult: " << hr << std::endl; DX_CHECK(-1, "pixel Shader Compiling Error ")
			}
			DX_RELEASE(errorBlob);
		}
		// Create hull shader
		{
			if (FAILED(D3DCompile(shaderText.c_str(), shaderText.size(), nullptr,
				&shaderMacros, nullptr, "HS", profiles.hs, 0, 0, &HS_Buffer, &errorBlob))) {
				std::cout << ((char*)errorBlob->GetBufferPointer()) << std::endl;
				DX_CHECK(-1, "hull Shader Compiling Error")
			}
			if (device->CreateHullShader(HS_Buffer->GetBufferPointer(), HS_Buffer->GetBufferSize(), NULL, &HS) != S_OK) {
				std::cout << "hresult: " << hr << std::endl; DX_CHECK(-1, "hull Shader Compiling Error ")
			}
			DX_RELEASE(errorBlob);
		}
		// Create domain shader
		{
			if (FAILED(D3DCompile(shaderText.c_str(), shaderText.size(), nullptr,
				&shaderMacros, nullptr, "DS", profiles.ds, 0, 0, &DS_Buffer, &errorBlob))) {
				std::cout << ((char*)errorBlob->GetBufferPointer()) << std::endl;
				DX_CHECK(-1, "domain Shader Compiling Error")
			}
			if (device->CreateDomainShader(DS_Buffer->GetBufferPointer(), DS_Buffer->GetBufferSize(), NULL, &DS) != S_OK) {
				std::cout << "hresult: " << hr << std::endl; DX_CHECK(-1, "domain Shader Compiling Error ")
			}
			DX_RELEASE(errorBlob);
		}
		
		PS_Buffer->Release();
		HS_Buffer->Release(); DS_Buffer->Release();
	}

	static Profiles GetLatestProfiles()
	{
		const D3D_FEATURE_LEVEL featureLevel = DirectxBackend::GetDevice()->GetFeatureLevel();
		assert(featureLevel >= D3D_FEATURE_LEVEL_11_0, "Shader model 5.0 is required, your GPU is not supporting tesellation");
		return { "vs_5_0", "ps_5_0", "ds_5_0", "hs_5_0"};
	}
};