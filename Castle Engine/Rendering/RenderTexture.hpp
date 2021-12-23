#pragma once
#include "../Helper.hpp"
#include "../Main/Event.hpp"

class RenderTexture
{
public:
	DXTexture2D* texture;
	DXShaderResourceView* textureView;
	DXRenderTargetView* renderTargetView;
	DXDepthStencilView* depthStencilView;
	DXTexture2D* depthStencilBuffer;
	Event OnSizeChanged;
private:
	DXDeviceContext* deviceContext;
	DXDevice* device;
	bool depth;
public:
	RenderTexture(const int& width, const int& height, const bool& createDepth = true);
	~RenderTexture() { Release(); }
	void Invalidate(const int&, const int&);
	void Release();
	void SetAsRendererTarget();
	void ClearRenderTarget(const float* bgColor);
	DXShaderResourceView* const GetShaderResourceView() const LAMBDAR(textureView);
};
