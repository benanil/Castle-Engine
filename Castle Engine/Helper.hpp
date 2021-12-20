// Directx Helper
#pragma once
#include <cstring>
#include <d3d11.h>
#include <cstdlib>

// helper type definitions
typedef ID3D11Device              DXDevice;
typedef ID3D11DeviceContext       DXDeviceContext;
typedef ID3D11RenderTargetView    DXRenderTargetView;
typedef ID3D11Buffer              DXBuffer;
typedef ID3D11Texture2D           DXTexture2D;
typedef ID3D11ShaderResourceView  DXTextureView;
typedef ID3D11SamplerState        DXTexSampler;
typedef ID3D11InputLayout         DXInputLayout;

// const char* to wchar_t*
static const wchar_t* GetWC(const char* c)
{
    const size_t cSize = strlen(c) + 1;
    wchar_t* wc = new wchar_t[cSize];
    mbstowcs(wc, c, cSize);
    return wc;
}

template<typename T>
inline static T DX_CREATE()
{
	T object{};
	std::memset(&object, 0, sizeof(T));

	return object;
}