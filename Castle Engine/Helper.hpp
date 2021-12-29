// Directx Helper
#pragma once
#include <cstring>
#include <d3d11.h>
#include <cstdlib>
#include <xnamath.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// helper type definitions
typedef ID3D11Device              DXDevice;
typedef ID3D11DeviceContext       DXDeviceContext;
typedef ID3D11RenderTargetView    DXRenderTargetView;
typedef ID3D11DepthStencilView    DXDepthStencilView;
typedef ID3D11DepthStencilState   DXDepthStencilState;
typedef ID3D11Buffer              DXBuffer;
typedef ID3D11Texture2D           DXTexture2D;
typedef ID3D11ShaderResourceView  DXShaderResourceView; // texture view
typedef ID3D11SamplerState        DXTexSampler;
typedef ID3D11InputLayout         DXInputLayout;
typedef ID3D10Blob                DXBlob;
typedef ID3D11VertexShader		  DXVertexShader;
typedef ID3D11PixelShader		  DXPixelShader;
typedef ID3D11RasterizerState     DXRasterizerState;
typedef ID3D11BlendState          DXBlendState;

#define DX_RELEASE(x) if (x) { x->Release(); x = nullptr ; }

// const char* to wchar_t*
static const wchar_t* GetWC(const char* c)
{
    const size_t cSize = strlen(c) + 1;
    wchar_t* wc = new wchar_t[cSize];
    mbstowcs(wc, c, cSize);
    return wc;
}
#define DX_CREATE(_type, name) _type name{}; memset(&name, 0,sizeof(_type));

#define LAMBDA(x) { x; }
#define LAMBDAR(x) { return x;  }

//math helpers
#include "Math.hpp"