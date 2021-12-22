#pragma once
#include "../Helper.hpp"

class RenderTexture
{
public:
	DXTexture2D* texture;
	DXShaderResourceView* textureView;
	DXRenderTargetView* renderTargetView;
	DXDepthStencilView* depthStencilView;
	DXTexture2D* depthStencilBuffer;
private:
	DXDeviceContext* deviceContext;
	DXDevice* device;
	bool depth;
public:
	RenderTexture(const int&, const int&, const bool& createDepth = true);
	~RenderTexture() { Release(); }
	void Invalidate(const int&, const int&);
	void Release();
	void SetAsRendererTarget();
	void ClearRenderTarget(const float*);
	DXShaderResourceView* const GetShaderResourceView() const LAMBDAR(textureView);
};
