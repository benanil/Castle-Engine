

#pragma once
#include "../Rendering.hpp"
#include "../Main/Event.hpp"

typedef Func<std::function<void(DXShaderResourceView*)>, DXShaderResourceView*> OnTextureChanged;

class RenderTexture
{
public:
	DXTexture2D* texture;
	DXShaderResourceView* textureView;
	DXRenderTargetView* renderTargetView;
	DXDepthStencilView* depthStencilView;
	DXTexture2D* depthStencilBuffer;
	OnTextureChanged OnSizeChanged;
	DXBlendState* blendState;
	UINT sampleCount;
private:
	DXDeviceContext* deviceContext;
	DXDevice* device;
	bool depth;
	DXGI_FORMAT format;
public:
	RenderTexture(const int& width, const int& height, const UINT& sampleCount, const bool& createDepth = true, DXGI_FORMAT format = DXGI_FORMAT_R16G16B16A16_FLOAT);
	~RenderTexture() { Release(); }
	void Invalidate(const int&, const int&);
	void Release();
	void SetBlendState();
	void SetAsRendererTarget();
	void ClearRenderTarget(const float* bgColor);
	DXShaderResourceView* const GetShaderResourceView() const LAMBDAR(textureView);
};
