#pragma once
#include "../Rendering.hpp"
#include <vector>
#include "Shader.hpp"
#include "Mesh.hpp"
#include "../FreeCamera.hpp"

// https://www.braynzarsoft.net/viewtutorial/q16390-20-cube-mapping-skybox

class Skybox
{
private:

	struct SkyboxVertex
	{
		glm::vec3 pos;
		
		static DXInputLayout* GetLayout(DXBlob* VS_Buffer)
		{
			DXInputLayout* vertLayout;
			
			D3D11_INPUT_ELEMENT_DESC layout[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 , D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};
			
			auto device = Engine::GetDevice();
			device->CreateInputLayout(layout, 1, VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), &vertLayout);
			return vertLayout;
		}
	};

	Shader shader;

	SphereCreateResult* mesh;

	DXBuffer* vertexBuffer;
	DXBuffer* indexBuffer;

	DXShaderResourceView* srv;
	DXDepthStencilState* DSLessEqual;
	DXRasterizerState* RSCullNone;
	DXTexSampler* textureSampler;

	DXInputLayout* vertexLayout;

public:
	Skybox(uint8_t LatLines, uint8_t LongLines, uint8_t samples) : shader(Shader("Skybox.hlsl\0", "Skybox.hlsl\0"))
	{
		mesh = CSCreateSphereVertexIndices(LatLines, LongLines);
		
		auto d3d11DevCon = Engine::GetDeviceContext();
		auto d3d11Device = Engine::GetDevice();
		
		CSCreateVertexIndexBuffers<SkyboxVertex, uint32_t>(
			d3d11Device, d3d11DevCon, 
			reinterpret_cast<SkyboxVertex*>(mesh->vertices), mesh->indices,
			mesh->vertexCount, mesh->indexCount, &vertexBuffer, &indexBuffer);

		vertexLayout = SkyboxVertex::GetLayout(shader.VS_Buffer);

		// loading cubemap
		D3DX11_IMAGE_LOAD_INFO loadSMInfo;
		loadSMInfo.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
		
		DXTexture2D* SMTexture = 0;

		DX_CHECK(
		D3DX11CreateTextureFromFile(d3d11Device, L"skymap.dds", &loadSMInfo, 0, (ID3D11Resource**)&SMTexture, 0), "skybox texture importing failed!")
		
		D3D11_TEXTURE2D_DESC SMTextureDesc;
		SMTexture->GetDesc(&SMTextureDesc);

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
		cmdesc.MultisampleEnable = true;
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

	void Draw(cbPerObject& cbPerObj, DXDeviceContext* DeviceContext, DXBuffer* constantBuffer, const FreeCamera* freeCamera)
	{
		XMMATRIX Model = XMMatrixScaling(900000, 900000, 900000);
		cbPerObj.MVP   = XMMatrixTranspose(Model * freeCamera->ViewProjection);
		cbPerObj.Model = XMMatrixTranspose(Model);
		
		DeviceContext->UpdateSubresource(constantBuffer, 0, NULL, &cbPerObj, 0, 0);
		DeviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);
		
		DeviceContext->PSSetShaderResources( 0, 1, &srv );
		DeviceContext->PSSetSamplers( 0, 1, &textureSampler);

		DeviceContext->OMSetDepthStencilState(DSLessEqual, 0);
		DeviceContext->RSSetState(RSCullNone);

		shader.Bind();
		
		DrawIndexedInfo drawInfo
		{
			DeviceContext, vertexLayout, 
			vertexBuffer, indexBuffer, mesh->indexCount
		};
		
		DrawIndexed32<SkyboxVertex>(&drawInfo);
	}

	~Skybox()
	{
		DX_RELEASE(DSLessEqual)
		DX_RELEASE(RSCullNone )
		DX_RELEASE(srv       )
		DX_RELEASE(vertexLayout)
		shader.Dispose();
		delete mesh;
	}
};



