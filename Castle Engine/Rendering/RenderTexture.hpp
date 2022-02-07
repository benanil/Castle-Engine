

#pragma once
#include "../Rendering.hpp"
#include "../Main/Event.hpp"
#include "../Helper.hpp"

typedef Func<std::function<void(DXShaderResourceView*)>, DXShaderResourceView*> OnTextureChanged;

enum class RenderTextureCreateFlags
{
	None   = 0,
	Depth  = 1,
	UAV    = 2,
	Linear = 4
};

CS_CREATE_ENUM_OPERATORS(RenderTextureCreateFlags)

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
	DXTexSampler* sampler;
	UINT sampleCount;
	int width, height;
private:
	DXDeviceContext* deviceContext;
	DXDevice* device;
	DXGI_FORMAT format;
	RenderTextureCreateFlags flags;
public:
	RenderTexture() {};
	RenderTexture(const int& _width, const int& _height, const UINT& sampleCount, 
		RenderTextureCreateFlags flags = RenderTextureCreateFlags::Depth, DXGI_FORMAT format = DXGI_FORMAT_R16G16B16A16_FLOAT);
	~RenderTexture() { Release(); }
	void Invalidate(int _width, int _height);
	void Release();
	void SetBlendState();
	void SetAsRendererTarget();
	void ClearRenderTarget(const float* bgColor);
	DXShaderResourceView* const GetShaderResourceView() const LAMBDAR(textureView);
};
