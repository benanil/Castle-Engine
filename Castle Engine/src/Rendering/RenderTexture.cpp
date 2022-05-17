
#include "RenderTexture.hpp"
#include "../Engine.hpp"
#include "../DirectxBackend.hpp"

RenderTexture::RenderTexture(
	const int& textureWidth, 
	const int& textureHeight,
	const UINT& _sampleCount,
	RenderTextureCreateFlags _flags,
	DXGI_FORMAT _format) : flags(_flags), sampleCount(_sampleCount), format(_format), width(textureWidth), height(textureHeight)
{
	deviceContext = DirectxBackend::GetDeviceContext();
	device        = DirectxBackend::GetDevice();

	DX_CREATE(D3D11_BLEND_DESC, blendDesc);
	blendDesc.AlphaToCoverageEnable = true;
	blendDesc.RenderTarget[0].BlendEnable = false;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	device->CreateBlendState(&blendDesc, &blendState);

	Invalidate(textureWidth, textureHeight);
}

void RenderTexture::SetAsRendererTarget()
{
	// Bind the render target view and depth stencil buffer to the output render pipeline.
	deviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
	SetBlendState();
}

void RenderTexture::BindTexture(UINT slot)
{
	deviceContext->PSSetShaderResources(slot, 1, &srv);
	deviceContext->PSSetSamplers(slot, 1, &sampler);
}

void RenderTexture::BindDepthTexture(UINT slot)
{
	deviceContext->PSSetShaderResources(slot, 1, &depthSRV);
	deviceContext->PSSetSamplers(slot, 1, &sampler);
}

void RenderTexture::BindDsvAndSetNullRenderTarget()
{
	// Set null render target because we are only going to draw
	// to depth buffer. Setting a null render target will disable
	// color writes.
	ID3D11RenderTargetView* renderTargets[1] = {0};
	deviceContext->OMSetRenderTargets(1, renderTargets, depthStencilView);
	deviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

using namespace CSCore; // for has flag

void RenderTexture::ClearRenderTarget(const float* colour) 
{
	// Clear the back buffer.
	deviceContext->ClearRenderTargetView(renderTargetView, colour);
	// clear depth buffer
	if (HasFlag(flags, RenderTextureCreateFlags::Depth))
	deviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);
}

void RenderTexture::SetBlendState()
{
	const float blend_factor[4] = { 0.f, 0.f, 0.f, 0.f };
	deviceContext->OMSetBlendState(blendState, blend_factor, 0xffffffff);
}

void RenderTexture::Invalidate(int _width, int _height)
{
	width = std::max(_width, 1);
	height = std::max(_height, 1);
	Release();
	
	bool isLinear = HasFlag(flags, RenderTextureCreateFlags::Linear);
	D3D11_FILTER filterMode = isLinear ? D3D11_FILTER_MIN_MAG_MIP_LINEAR : D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc{};
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResViewDesc{};
	shaderResViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResViewDesc.Texture2D.MipLevels = 1;

	DX_CREATE(D3D11_TEXTURE2D_DESC, textureDesc);

	// create sampler
	if (!sampler)
	{
		DX_CREATE(D3D11_SAMPLER_DESC, sampDesc);
		sampDesc.Filter = filterMode;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = isLinear ? D3D11_FLOAT32_MAX : 1;
		device->CreateSamplerState(&sampDesc, &sampler);
	}
	
	if (!HasFlag(flags, RenderTextureCreateFlags::NoColor))
	{
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = format;
		textureDesc.SampleDesc.Count = std::max(sampleCount, 1u);
		textureDesc.SampleDesc.Quality = sampleCount;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;
		
		if (HasFlag(flags, RenderTextureCreateFlags::UAV)){
			textureDesc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
		} 
		DX_CHECK(
		device->CreateTexture2D(&textureDesc, NULL, &texture), "render texture creation failed!" );	

		renderTargetViewDesc.Format = textureDesc.Format;
		renderTargetViewDesc.ViewDimension = sampleCount > 1 ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;
		renderTargetViewDesc.Texture2D.MipSlice = 0;
		
		DX_CHECK(
		device->CreateRenderTargetView(texture, &renderTargetViewDesc, &renderTargetView), "render target view generation failed!");
		
		shaderResViewDesc.Format = textureDesc.Format;
		shaderResViewDesc.ViewDimension = sampleCount > 1 ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
		
		DX_CHECK(
		device->CreateShaderResourceView(texture, &shaderResViewDesc, &textureView), "render target view creation failed!");
	}

	if (HasFlag(flags, RenderTextureCreateFlags::Depth))
	{
		DX_CREATE(D3D11_TEXTURE2D_DESC, depthDesc)
		depthDesc.Width = width;
		depthDesc.Height = height;
		depthDesc.MipLevels = 1;
		depthDesc.ArraySize = 1;
		depthDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		depthDesc.SampleDesc.Count = std::max(sampleCount, 1u);
		depthDesc.SampleDesc.Quality = sampleCount;
		depthDesc.Usage = D3D11_USAGE_DEFAULT;
		depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		
		DX_CHECK(
		device->CreateTexture2D(&depthDesc, NULL, &depthStencilBuffer), "failed to create depth texture")

		DX_CREATE(D3D11_DEPTH_STENCIL_VIEW_DESC, dsvDesc);
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.ViewDimension = sampleCount > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
		DX_CHECK(
		device->CreateDepthStencilView(depthStencilBuffer, &dsvDesc, &depthStencilView), "Failed to Create depth stencil for render texture")
		
		shaderResViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		shaderResViewDesc.ViewDimension = sampleCount > 1 ?  D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
		DX_CHECK(
		device->CreateShaderResourceView(depthStencilBuffer, &shaderResViewDesc, &depthSRV), "depth srv creation failed!");
	}

	OnSizeChanged.Invoke(textureView);
}

void RenderTexture::Release()
{
	DX_RELEASE(texture);
	DX_RELEASE(textureView);
	DX_RELEASE(renderTargetView) ;
	DX_RELEASE(depthSRV);
	DX_RELEASE(depthStencilView);
	DX_RELEASE(depthStencilBuffer);
	DX_RELEASE(blendState);
	DX_RELEASE(sampler);
}

