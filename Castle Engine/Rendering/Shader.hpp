#pragma once
#include "../Helper.hpp"
#include "../Engine.hpp"
#include <cassert>
#include <iostream>
#include <D3DX11.h>

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

	Shader(const wchar_t* vertPath, const wchar_t* PSPath, const char* vertName = "VS", const char* PSName = "PS")
	{
		Profiles profiles = GetLatestProfiles();

		DX_CHECK(
			D3DX11CompileFromFile(vertPath, 0, 0, vertName, profiles.vertex, 0, 0, 0, &VS_Buffer, 0, 0),
			"VertexShader Compiling error! \n");
		
		DX_CHECK(
			D3DX11CompileFromFile(PSPath, 0, 0, PSName, profiles.frag, 0, 0, 0, &PS_Buffer, 0, 0),
			"FragShader Compiling error! \n");
		
		
		const auto device = Engine::GetDevice();
		//Create the Shader Objects
		DX_CHECK(
			device->CreateVertexShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), NULL, &VS),
			"VertexShader Compiling error! \n");
		
		DX_CHECK(
			device->CreatePixelShader(PS_Buffer->GetBufferPointer(), PS_Buffer->GetBufferSize(), NULL, &PS),
			"FragShader Compiling error! \n");
	}

	void Bind(DXDeviceContext* d3d11DevCon)
	{
		//Set Vertex and Pixel Shaders
		d3d11DevCon->VSSetShader(VS, 0, 0);
		d3d11DevCon->PSSetShader(PS, 0, 0);	
	}

	~Shader()
	{
		DX_RELEASE(VS);
		DX_RELEASE(PS);
		DX_RELEASE(VS_Buffer);
		DX_RELEASE(PS_Buffer);
	}	

private:
	static Profiles GetLatestProfiles()
	{
		const D3D_FEATURE_LEVEL featureLevel = Engine::GetDevice()->GetFeatureLevel();
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