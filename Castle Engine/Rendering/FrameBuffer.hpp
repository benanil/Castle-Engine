#pragma once
#include <D3DX11.h>
#include <wrl/client.h>
#include <stdexcept>
#include "../Engine.hpp"

using Microsoft::WRL::ComPtr;

struct FrameBuffer
{
	ComPtr<ID3D11Texture2D> colorTexture;
	ComPtr<ID3D11Texture2D> depthStencilTexture;
	ComPtr<ID3D11RenderTargetView> rtv;
	ComPtr<ID3D11ShaderResourceView> srv;
	ComPtr<ID3D11DepthStencilView> dsv;
	UINT width, height;
	UINT samples;

	ID3D11DeviceContext* deviceContext;
	bool depth;

	void SetAsRendererTarget()
	{
		// Bind the render target view and depth stencil buffer to the output render pipeline.
		deviceContext->OMSetRenderTargets(1, rtv.GetAddressOf(), dsv.Get());
	}

	void ClearRenderTarget(const float* colour)
	{
		// Clear the back buffer.
		deviceContext->ClearRenderTargetView(rtv.Get(), colour);
		// clear depth buffer
		if (depth)
			deviceContext->ClearDepthStencilView(dsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);
	}

	FrameBuffer(UINT width, UINT height, UINT samples, DXGI_FORMAT colorFormat, DXGI_FORMAT depthstencilFormat) 
	{
		width = width;
		height = height;
		samples = samples;
		
		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.SampleDesc.Count = samples;

		auto m_device = Engine::GetDevice();

		if (colorFormat != DXGI_FORMAT_UNKNOWN) {
			desc.Format = colorFormat;
			desc.BindFlags = D3D11_BIND_RENDER_TARGET;
			if (samples <= 1) {
				desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
			}
			if (FAILED(m_device->CreateTexture2D(&desc, nullptr, &colorTexture))) {
				throw std::runtime_error("Failed to create FrameBuffer color texture");
			}

			D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.Format = desc.Format;
			rtvDesc.ViewDimension = (samples > 1) ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;
			if (FAILED(m_device->CreateRenderTargetView(colorTexture.Get(), &rtvDesc, &rtv))) {
				throw std::runtime_error("Failed to create FrameBuffer render target view");
			}

			if (samples <= 1) {
				D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
				srvDesc.Format = desc.Format;
				srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Texture2D.MostDetailedMip = 0;
				srvDesc.Texture2D.MipLevels = 1;
				if (FAILED(m_device->CreateShaderResourceView(colorTexture.Get(), &srvDesc, &srv))) {
					throw std::runtime_error("Failed to create FrameBuffer shader resource view");
				}
			}
		}

		if (depthstencilFormat != DXGI_FORMAT_UNKNOWN) {
			depth = true;
			desc.Format = depthstencilFormat;
			desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			if (FAILED(m_device->CreateTexture2D(&desc, nullptr, &depthStencilTexture))) {
				throw std::runtime_error("Failed to create FrameBuffer depth-stencil texture");
			}

			D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
			dsvDesc.Format = desc.Format;
			dsvDesc.ViewDimension = (samples > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
			if (FAILED(m_device->CreateDepthStencilView(depthStencilTexture.Get(), &dsvDesc, &dsv))) {
				throw std::runtime_error("Failed to create FrameBuffer depth-stencil view");
			}
		}
	}

};