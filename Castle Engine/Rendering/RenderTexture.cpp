#include "RenderTexture.hpp"
#include "../Engine.hpp"

RenderTexture::RenderTexture(
	const int& textureWidth, 
	const int& textureHeight, 
	const bool& createDepth) : depth(createDepth)
{
	deviceContext = Engine::GetDeviceContext();
	device        = Engine::GetDevice();
	Invalidate(textureWidth, textureHeight);
}

void RenderTexture::SetAsRendererTarget()
{
	// Bind the render target view and depth stencil buffer to the output render pipeline.
	deviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
}

void RenderTexture::ClearRenderTarget(const float* colour) 
{
	// Clear the back buffer.
	deviceContext->ClearRenderTargetView(renderTargetView, colour);
	// clear depth buffer
	if (depthStencilView)
	deviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);
}

void RenderTexture::Invalidate(const int& width, const int& height)
{
	Release();

	DX_CREATE(D3D11_TEXTURE2D_DESC, textureDesc)
	
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	
	DX_CHECK(
		device->CreateTexture2D(&textureDesc, NULL, &texture), "render texture creation failed!"
	);
	
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc{};
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;
	
	DX_CHECK(
		device->CreateRenderTargetView(texture, &renderTargetViewDesc, &renderTargetView), "render target view generation failed!"
	);
	
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResViewDesc{};
	shaderResViewDesc.Format = textureDesc.Format;
	shaderResViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResViewDesc.Texture2D.MipLevels = 1;
	
	DX_CHECK(
		device->CreateShaderResourceView(texture, &shaderResViewDesc, &textureView), "render target view creation failed!"
	);
	
	if (depth)
	{
		D3D11_TEXTURE2D_DESC depthDesc{};
		depthDesc.MipLevels = 1;
		depthDesc.ArraySize = 1;
		depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthDesc.SampleDesc.Count = 1;
		depthDesc.SampleDesc.Quality = 0;
		depthDesc.Usage = D3D11_USAGE_DEFAULT;
		depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthDesc.CPUAccessFlags = 0;
		depthDesc.MiscFlags = 0;
		
		device->CreateTexture2D(&depthDesc, NULL, &depthStencilBuffer);
		device->CreateDepthStencilView(depthStencilBuffer, NULL, &depthStencilView);
	}
}

void RenderTexture::Release()
{
	DX_RELEASE(texture) 
	DX_RELEASE(textureView) 
	DX_RELEASE(renderTargetView) 
	DX_RELEASE(depthStencilView)
	DX_RELEASE(depthStencilBuffer)
}

