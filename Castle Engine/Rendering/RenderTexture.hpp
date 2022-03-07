

#pragma once
#include "../Rendering.hpp"
#include "../Main/Event.hpp"
#include "../Helper.hpp"

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
	ID3D11Texture2D* texture;
	
	union  {
		ID3D11ShaderResourceView* textureView;
		ID3D11ShaderResourceView* srv;
	};
	
	ID3D11ShaderResourceView* depthSRV;
	ID3D11RenderTargetView* renderTargetView;
	ID3D11DepthStencilView* depthStencilView;
	ID3D11Texture2D* depthStencilBuffer;
	EventEmitter<DXShaderResourceView*> OnSizeChanged;
	DXBlendState* blendState;
	DXTexSampler* sampler;
	UINT sampleCount;
	int width, height;
	RenderTextureCreateFlags flags;
private:
	DXDeviceContext* deviceContext;
	DXDevice* device;
	DXGI_FORMAT format;
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
