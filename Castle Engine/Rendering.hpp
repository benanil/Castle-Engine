// Directx Helper
#pragma once
#include <cstring>
#include <d3d11.h>
#include <cstdlib>
#include <stdexcept>
#include <cassert>
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
#define DX_CREATE(_type, _name) _type _name{}; memset(&_name, 0,sizeof(_type));

// const char* to wchar_t*
static const wchar_t* GetWC(const char* c)
{
	const size_t cSize = strlen(c) + 1;
	wchar_t* wc = new wchar_t[cSize];
	mbstowcs(wc, c, cSize);
	return wc;
}
template<typename VertexT, typename IndexT>
static inline void CSCreateVertexIndexBuffers(
	DXDevice* d3d11Device,
	const VertexT* vertices,
	const IndexT* indices,
	uint32_t vertexCount, uint32_t indexCount,
	DXBuffer** p_vertexBuffer,
	DXBuffer** p_indexBuffer)
{
	// create vertex buffer		
	DX_CREATE(D3D11_BUFFER_DESC, vertexBufferDesc);

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexT) * vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	DX_CREATE(D3D11_SUBRESOURCE_DATA, vertexBufferData);

	vertexBufferData.pSysMem = vertices;
	if (FAILED(d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, p_vertexBuffer)))
	{
		throw std::runtime_error("vertex buffer creation failed!");
	}
	// create index buffer
	DX_CREATE(D3D11_BUFFER_DESC, indexDesc);
	indexDesc.Usage = D3D11_USAGE_DEFAULT;
	indexDesc.ByteWidth = sizeof(IndexT) * indexCount;
	indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = indices;
	d3d11Device->CreateBuffer(&indexDesc, &initData, p_indexBuffer);
}

struct DrawIndexedInfo
{
	DXDeviceContext* d3d11DevCon;
	DXInputLayout* vertexLayout;
	DXBuffer* vertexBuffer; DXBuffer* indexBuffer; uint32_t indexCount;
};

template<typename TVertex>
void inline DrawIndexed32(DrawIndexedInfo* drawInfo)
{
	drawInfo->d3d11DevCon->IASetInputLayout(drawInfo->vertexLayout);
	drawInfo->d3d11DevCon->IASetIndexBuffer(drawInfo->indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	UINT stride = sizeof(TVertex), offset = 0;
	drawInfo->d3d11DevCon->IASetVertexBuffers(0, 1, &drawInfo->vertexBuffer, &stride, &offset);
	drawInfo->d3d11DevCon->DrawIndexed(drawInfo->indexCount, 0, 0);
}

template<typename TVertex>
void inline DrawIndexed16(DrawIndexedInfo* drawInfo)
{
	drawInfo->d3d11DevCon->IASetInputLayout(drawInfo->vertexLayout);
	drawInfo->d3d11DevCon->IASetIndexBuffer(drawInfo->indexBuffer, DXGI_FORMAT_R16_UINT, 0);

	// draw skybox sphere
	UINT stride = sizeof(TVertex), offset = 0;
	drawInfo->d3d11DevCon->IASetVertexBuffers(0, 1, &drawInfo->vertexBuffer, &stride, &offset);

	drawInfo->d3d11DevCon->DrawIndexed(drawInfo->indexCount, 0, 0);
}

template<typename T>
inline void DXCreateConstantBuffer(ID3D11Device* device, ID3D11Buffer*& buffer, T* initialData = nullptr)
{
	DX_CREATE(D3D11_BUFFER_DESC, cbUniformDesc);
	cbUniformDesc.Usage = D3D11_USAGE_DEFAULT;
	cbUniformDesc.ByteWidth = sizeof(T);
	cbUniformDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	DX_CREATE(D3D11_SUBRESOURCE_DATA, vinitData);
	vinitData.pSysMem = initialData;
	D3D11_SUBRESOURCE_DATA* p_vinit = initialData ? &vinitData : nullptr;
	if (FAILED(device->CreateBuffer(&cbUniformDesc, p_vinit, &buffer))) {
		assert(0, "Constant Buffer Creation Failed!");
	}
}

template<typename T>
void inline DXCreateStructuredBuffer(ID3D11Device* device, ID3D11Buffer*& buffer, unsigned int numElements, T* initialData = nullptr)
{
	DX_CREATE(D3D11_BUFFER_DESC, RWBufferDesc);
	RWBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	RWBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	RWBufferDesc.StructureByteStride = sizeof(T);
	RWBufferDesc.ByteWidth = sizeof(T) * numElements;
	RWBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	RWBufferDesc.CPUAccessFlags = 0;

	DX_CREATE(D3D11_SUBRESOURCE_DATA, vinitData);
	vinitData.pSysMem = initialData;
	D3D11_SUBRESOURCE_DATA* p_vinitData = initialData ? &vinitData : nullptr;
	ID3D11Buffer* GpuBuffer;
	if (FAILED(device->CreateBuffer(&RWBufferDesc, p_vinitData, &GpuBuffer))) { assert(0, "strutcured buffer creation failed"); };
}


#define LAMBDA(x) { x; }
#define LAMBDAR(x) { return x;  }
