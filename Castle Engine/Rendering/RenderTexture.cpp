#include "RenderTexture.hpp"
#include "../Engine.hpp"

RenderTexture::RenderTexture(
	const int& textureWidth, 
	const int& textureHeight,
	const UINT& _sampleCount,
	const bool& createDepth,
	DXGI_FORMAT _format) : depth(createDepth), sampleCount(_sampleCount), format(_format)
{
	deviceContext = Engine::GetDeviceContext();
	device        = Engine::GetDevice();

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

void RenderTexture::ClearRenderTarget(const float* colour) 
{
	// Clear the back buffer.
	deviceContext->ClearRenderTargetView(renderTargetView, colour);
	// clear depth buffer
	if (depth)
	deviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);
}

void RenderTexture::SetBlendState()
{
	const float blend_factor[4] = { 0.f, 0.f, 0.f, 0.f };
	deviceContext->OMSetBlendState(blendState, blend_factor, 0xffffffff);
}

void RenderTexture::Invalidate(const int& width, const int& height)
{
	Release();

	DX_CREATE(D3D11_TEXTURE2D_DESC, textureDesc)
	
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
	
	DX_CHECK(
	device->CreateTexture2D(&textureDesc, NULL, &texture), "render texture creation failed!" );

	DX_CREATE(D3D11_SAMPLER_DESC, sampDesc);
	
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;    
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = 1;
	device->CreateSamplerState( &sampDesc, &sampler);
	
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc{};
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = sampleCount > 1 ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;

	renderTargetViewDesc.Texture2D.MipSlice = 0;
	
	DX_CHECK(
	device->CreateRenderTargetView(texture, &renderTargetViewDesc, &renderTargetView), "render target view generation failed!");
	
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResViewDesc{};
	shaderResViewDesc.Format = textureDesc.Format;
	shaderResViewDesc.ViewDimension = sampleCount > 1 ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResViewDesc.Texture2D.MipLevels = 1;
	
	DX_CHECK(
	device->CreateShaderResourceView(texture, &shaderResViewDesc, &textureView), "render target view creation failed!");

	if (depth)
	{
		DX_CREATE(D3D11_TEXTURE2D_DESC, depthDesc)
		depthDesc.Width = width;
		depthDesc.Height = height;
		depthDesc.MipLevels = 1;
		depthDesc.ArraySize = 1;
		depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthDesc.SampleDesc.Count = std::max(sampleCount, 1u);
		depthDesc.SampleDesc.Quality = sampleCount;
		depthDesc.Usage = D3D11_USAGE_DEFAULT;
		depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthDesc.CPUAccessFlags = 0;
		depthDesc.MiscFlags = 0;
		
		DX_CHECK(
		device->CreateTexture2D(&depthDesc, NULL, &depthStencilBuffer), "failed to create depth texture")

		DX_CHECK(
		device->CreateDepthStencilView(depthStencilBuffer, NULL, &depthStencilView), "Failed to Create depth stencil for render texture")
	}
	
	OnSizeChanged(textureView);
}

void RenderTexture::Release()
{
	DX_RELEASE(texture) 
	DX_RELEASE(textureView) 
	DX_RELEASE(renderTargetView) 
	DX_RELEASE(depthStencilView)
	DX_RELEASE(depthStencilBuffer)
}

