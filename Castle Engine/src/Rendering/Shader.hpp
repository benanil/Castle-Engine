#pragma once
#include "../Rendering.hpp"
#include "../DirectxBackend.hpp"
#include "../Helper.hpp"
#include "../Engine.hpp"
#include <cassert>
#include <D3DX11.h>
#include <iostream>

#include <d3dcompiler.h>
#ifdef _MSC_VER
#pragma comment(lib, "d3dcompiler") // Automatically link with d3dcompiler.lib as we are using D3DCompile() below.
#endif

class Shader
{
	struct Profiles 
	{
		const char* vertex;
		const char* frag; 
	};

public:
	DXVertexShader* VS;
	DXPixelShader* PS ;
	DXBlob* VS_Buffer ;
	DXBlob* PS_Buffer ;

public:

	Shader() {};
	Shader(const char* path) 
	{
		Load(path, path, "VS", "PS");
	}
	Shader(const char* vertPath, const char* PSPath, const char* vertName = "VS", const char* PSName = "PS")
	{
		Load(vertPath, PSPath, vertName, PSName);
	}

	void Bind()
	{
		//Set Vertex and Pixel Shaders
		d3d11DevCon->VSSetShader(VS, 0, 0);
		d3d11DevCon->PSSetShader(PS, 0, 0);	
	}

	void Dispose()
	{
		DX_RELEASE(VS);
		DX_RELEASE(PS);
		DX_RELEASE(VS_Buffer);
		DX_RELEASE(PS_Buffer);
	}

	~Shader() {
		Dispose();
	}	

private:

	DXDeviceContext* d3d11DevCon;

	void Load(const char* vertPath, const char* PSPath, const char* vertName = "VS", const char* PSName = "PS")
	{
		Profiles profiles = GetLatestProfiles();
		
		DXDevice* device = DirectxBackend::GetDevice();
		d3d11DevCon = DirectxBackend::GetDeviceContext();
		
		std::string vertexShader   = ReadAllText(std::string(vertPath));
		std::string fragmentShader = ReadAllText(std::string(PSPath));
		
		DXBlob* vertexErrorBlob, *fragErrorBlob;
		
#ifdef  DEBUG
		D3D_SHADER_MACRO shaderMacros[] = { {"DEBUG", "1"}, {nullptr, nullptr}};
#else 
		D3D_SHADER_MACRO shaderMacros[] = { {"RELEASE", "1"}, {nullptr, nullptr} };
#endif //  DEBUG

		if (FAILED(
			D3DCompile(vertexShader.c_str(), vertexShader.size(), nullptr,
			shaderMacros, nullptr, "VS", profiles.vertex, 0, 0, &VS_Buffer, &vertexErrorBlob)))
		{
			std::cout << "Vertex Shader Compiling Error:\n" << ((char*)vertexErrorBlob->GetBufferPointer()) << std::endl;
			DX_CHECK(-1, "Vertex Shader Compiling Error")
		}
		
		HRESULT hr = device->CreateVertexShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), NULL, &VS);
		
		if (hr != S_OK) {
			std::cout << "vertex Shader compiling error!\n hresult: " << hr << std::endl;
			DX_CHECK(-1, "vertex Shader Compiling Error ")
		}
		
		DX_RELEASE(vertexErrorBlob);
		
		if (FAILED(
			D3DCompile(fragmentShader.c_str(), fragmentShader.size(), nullptr,
			shaderMacros, nullptr, PSName, profiles.frag, 0, 0, &PS_Buffer, &fragErrorBlob)))
		{
			std::cout << "pixel Shader Compiling Error:\n" << ((char*)fragErrorBlob->GetBufferPointer()) << std::endl;
			DX_CHECK(-1, "pixel Shader Compiling Error")
		}
		
		hr = device->CreatePixelShader(PS_Buffer->GetBufferPointer(), PS_Buffer->GetBufferSize(), NULL, &PS);
		
		if (hr != S_OK) {
			std::cout << "Pixel Shader compiling error!\n hresult: " << hr << std::endl;
			DX_CHECK(-1, "pixel Shader Compiling Error ")
		}
		DX_RELEASE(fragErrorBlob);
	}

	static Profiles GetLatestProfiles()
	{
		const D3D_FEATURE_LEVEL featureLevel = DirectxBackend::GetDevice()->GetFeatureLevel();
		switch (featureLevel)
		{
			case D3D_FEATURE_LEVEL_11_0: return {"vs_5_0"          , "ps_5_0"          };
			case D3D_FEATURE_LEVEL_10_1: return {"vs_4_1"          , "ps_4_1"          };
			case D3D_FEATURE_LEVEL_10_0: return {"vs_4_0"          , "ps_4_0"          };
			case D3D_FEATURE_LEVEL_9_3:  return {"vs_4_0_level_9_3", "ps_4_0_level_9_3"};
			case D3D_FEATURE_LEVEL_9_1:  return {"vs_4_0_level_9_1", "ps_4_0_level_9_1"};
			default: return {"vs_5_0", "ps_5_0"};
		}
	}
};