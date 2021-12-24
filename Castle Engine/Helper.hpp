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

#define XM_GET_XYZ(x) XMGETX(x), XMGETY(x), XMGETZ(x)

// NE means no except
#define NELAMBDA(x) noexcept { x;  }
#define NELAMBDAR(x) noexcept { return x;  }

#define DX_RAD_TO_DEG 57.2958f
#define DX_DEG_TO_RAD 0.017453f
#define GLM_GET_XYZ(vec) vec.x, vec.y, vec.z 
#define DX_INLINE [[nodiscard]] __forceinline 

typedef XMVECTORF32 xmVector;
typedef XMMATRIX xmMatrix;

DX_INLINE xmVector GlmToXM(const glm::vec3& vec) NELAMBDAR({GLM_GET_XYZ(vec)})

DX_INLINE float* XMPTR(xmVector& vector)
{
	return reinterpret_cast<float*>(&vector);
}

DX_INLINE float* XMPTR(xmMatrix& vector)
{
	return &vector._11;
}

DX_INLINE const float& XMGETX(const xmVector& vector) LAMBDAR(vector.f[0])
DX_INLINE const float& XMGETY(const xmVector& vector) LAMBDAR(vector.f[1])
DX_INLINE const float& XMGETZ(const xmVector& vector) LAMBDAR(vector.f[2])
DX_INLINE const float& XMGETW(const xmVector& vector) LAMBDAR(vector.f[3])

DX_INLINE float XM_RadToDeg(const float& radians) NELAMBDAR(radians * DX_RAD_TO_DEG)
DX_INLINE float XM_DegToRad(const float& degree)  NELAMBDAR(degree * DX_DEG_TO_RAD)

// xmath euler
DX_INLINE void XM_DegToRad(const xmVector& radians, xmVector& degree) noexcept
{
	degree.f[0] = XMGETX(radians) * DX_DEG_TO_RAD;
	degree.f[1] = XMGETY(radians) * DX_DEG_TO_RAD;
	degree.f[2] = XMGETZ(radians) * DX_DEG_TO_RAD;
}

DX_INLINE void XM_RadToDeg(const xmVector& radians, xmVector& degree) noexcept
{
	degree.f[0] = XMGETX(radians) * DX_RAD_TO_DEG;
	degree.f[1] = XMGETY(radians) * DX_RAD_TO_DEG;
	degree.f[2] = XMGETZ(radians) * DX_RAD_TO_DEG;
}

DX_INLINE xmVector XM_DegToRad(const xmVector& radians) noexcept
{
	xmVector degree{};
	degree.f[0] = XMGETX(radians) * DX_DEG_TO_RAD;
	degree.f[1] = XMGETY(radians) * DX_DEG_TO_RAD;
	degree.f[2] = XMGETZ(radians) * DX_DEG_TO_RAD;
	return std::move(degree);
}

DX_INLINE xmVector XM_RadToDeg(const xmVector& radians) noexcept
{
	xmVector degree{};
	degree.f[0] = XMGETX(radians) * DX_RAD_TO_DEG;
	degree.f[1] = XMGETY(radians) * DX_RAD_TO_DEG;
	degree.f[2] = XMGETZ(radians) * DX_RAD_TO_DEG;
	return std::move(degree);
}

// GLM EULER

DX_INLINE void GLM_DegToRad(const glm::vec3& radians, glm::vec3& degree) noexcept
{
	degree.x = radians.x * DX_DEG_TO_RAD;
	degree.y = radians.y * DX_DEG_TO_RAD;
	degree.z = radians.z * DX_DEG_TO_RAD;
}

DX_INLINE void GLM_RadToDeg(const glm::vec3& radians, glm::vec3& degree) noexcept
{
	degree.x = radians.x * DX_RAD_TO_DEG;
	degree.y = radians.y * DX_RAD_TO_DEG;
	degree.z = radians.z * DX_RAD_TO_DEG;
}

DX_INLINE glm::vec3 GLM_RadToDeg(const glm::vec3& radians) noexcept
{
	glm::vec3 degree{};
	degree.x = radians.x * DX_RAD_TO_DEG;
	degree.y = radians.y * DX_RAD_TO_DEG;
	degree.z = radians.z * DX_RAD_TO_DEG;
	return std::move(degree);
}

DX_INLINE glm::vec3 GLM_DegToRad(const glm::vec3& radians) noexcept
{
	glm::vec3 degree{};
	degree.x = radians.x * DX_DEG_TO_RAD;
	degree.y = radians.y * DX_DEG_TO_RAD;
	degree.z = radians.z * DX_DEG_TO_RAD;
	return std::move(degree);
}

#undef NELAMBDAR
#undef NELAMBDA