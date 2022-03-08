#include "FXAA.hpp"
#include "Shader.hpp"
#include "../DirectxBackend.hpp"
#include "../Rendering.hpp"
#include "Renderer3D.hpp"
#include <array>

namespace FXAA
{
	struct FXAACBuffer {
		int width, height, padding0, padding1;
	} fxaaCBuffer;
	Shader* shader;
	Shader* FlatShader; // only texture
	RenderTexture* FXAARenderTexture;
	ID3D11Buffer* cbuffer;
	ID3D11Device* Device;
	ID3D11DeviceContext* DeviceContext;
}

void FXAA::Initialize()
{
	Device = DirectxBackend::GetDevice(); DeviceContext = DirectxBackend::GetDeviceContext();
	shader = new Shader("Shaders/FXAA.hlsl");
	FlatShader = new Shader("Shaders/FlatShader.hlsl");
	FXAARenderTexture = new RenderTexture(32, 32, DirectxBackend::GetMSAASamples(), RenderTextureCreateFlags::Linear, DXGI_FORMAT_R8G8B8A8_UNORM);
	DXCreateConstantBuffer(Device, cbuffer, &fxaaCBuffer);
}

void FXAA::Proceed(RenderTexture& RenderTexture)
{
	if (RenderTexture.width != FXAARenderTexture->width ||
		RenderTexture.height != FXAARenderTexture->height)
	{
		fxaaCBuffer.width = RenderTexture.width; 
		fxaaCBuffer.height = RenderTexture.height;
		DeviceContext->UpdateSubresource(cbuffer, 0, NULL, &fxaaCBuffer, 0, 0);
		FXAARenderTexture->Invalidate(RenderTexture.width, RenderTexture.height);
	}

	shader->Bind();
	FXAARenderTexture->SetAsRendererTarget();

	std::array<DXShaderResourceView*, 1> SRVs = { RenderTexture.textureView };
	std::array<DXTexSampler*, 1> samplers = { RenderTexture.sampler };

	DeviceContext->PSSetConstantBuffers(0, 1, &cbuffer);
	DeviceContext->PSSetShaderResources(0, SRVs.size(), SRVs.data());
	DeviceContext->PSSetSamplers(0, samplers.size(), samplers.data());
	
	Renderer3D::RenderToQuad();

	FlatShader->Bind();
	RenderTexture.SetAsRendererTarget();
	SRVs[0] = FXAARenderTexture->srv;
	samplers[0] = FXAARenderTexture->sampler;
	DeviceContext->PSSetShaderResources(0, SRVs.size(), SRVs.data());
	DeviceContext->PSSetSamplers(0, samplers.size(), samplers.data());
	
	Renderer3D::RenderToQuad();
}
