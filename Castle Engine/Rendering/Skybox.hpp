#pragma once
#include "../Helper.hpp"
#include <vector>
#include "Shader.hpp"
#include "Mesh.hpp"

// https://www.braynzarsoft.net/viewtutorial/q16390-20-cube-mapping-skybox

class Skybox
{
public:
	Skybox(uint8_t LatLines, uint8_t LongLines, uint8_t  samples) 
		: mesh(LatLines, LongLines), shader(Shader("Skybox.hlsl\0", "Skybox.hlsl\0"))
	{
		auto d3d11DevCon = Engine::GetDeviceContext();
		auto d3d11Device = Engine::GetDevice();

		vertexLayout = SkyboxVertex::GetLayout(shader.VS_Buffer);

		// loading cubemap
		D3DX11_IMAGE_LOAD_INFO loadSMInfo;
		loadSMInfo.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
		
		DXTexture2D* SMTexture = 0;

		DX_CHECK(
		D3DX11CreateTextureFromFile(d3d11Device, L"skymap.dds", &loadSMInfo, 0, (ID3D11Resource**)&SMTexture, 0), "skybox texture importing failed!")
		
		D3D11_TEXTURE2D_DESC SMTextureDesc;
		SMTexture->GetDesc(&SMTextureDesc);

		SMTextureDesc.SampleDesc.Count = samples;
		SMTextureDesc.SampleDesc.Quality = samples;

		D3D11_SHADER_RESOURCE_VIEW_DESC SMViewDesc;
		SMViewDesc.Format = SMTextureDesc.Format;
		SMViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		SMViewDesc.TextureCube.MipLevels = SMTextureDesc.MipLevels;
		SMViewDesc.TextureCube.MostDetailedMip = 0;
		
		DX_CHECK(
		d3d11Device->CreateShaderResourceView(SMTexture, &SMViewDesc, &srv), "shader resource creation failed!")

		DX_CREATE(D3D11_SAMPLER_DESC, sampDesc);

		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;    
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		d3d11Device->CreateSamplerState( &sampDesc, &textureSampler );

		// create render states
		DX_CREATE(D3D11_RASTERIZER_DESC, cmdesc);
		cmdesc.FillMode = D3D11_FILL_SOLID;
		cmdesc.CullMode = D3D11_CULL_BACK;
		cmdesc.FrontCounterClockwise = true;
		cmdesc.CullMode = D3D11_CULL_NONE;
		d3d11Device->CreateRasterizerState(&cmdesc, &RSCullNone);
		
		DX_CREATE(D3D11_DEPTH_STENCIL_DESC, dssDesc);
		dssDesc.DepthEnable = true;
		dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dssDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		
		d3d11Device->CreateDepthStencilState(&dssDesc, &DSLessEqual);
	}

	void Draw()
	{
		auto d3d11DevCon =  Engine::GetDeviceContext();
		
		d3d11DevCon->PSSetShaderResources( 0, 1, &srv );
		d3d11DevCon->PSSetSamplers( 0, 1, &textureSampler);

		d3d11DevCon->OMSetDepthStencilState(DSLessEqual, 0);
		d3d11DevCon->RSSetState(RSCullNone);

		d3d11DevCon->IASetInputLayout(vertexLayout);

		shader.Bind(d3d11DevCon);
		mesh.Draw();

	}

	~Skybox()
	{
		DX_RELEASE(DSLessEqual)
		DX_RELEASE(RSCullNone )
		DX_RELEASE(srv       )
		DX_RELEASE(vertexLayout)
		shader.Dispose();
		mesh.Dispose();
	}

private:

	SphereSky mesh;
	Shader shader;
	
	DXShaderResourceView* srv           ;
	DXDepthStencilState*  DSLessEqual   ;
	DXRasterizerState*    RSCullNone    ;
	DXTexSampler*         textureSampler;

	DXInputLayout* vertexLayout;

	int NumSphereVertices;
	int NumSphereFaces;
};



