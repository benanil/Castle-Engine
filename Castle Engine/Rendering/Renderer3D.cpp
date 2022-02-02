#pragma once
#include "Renderer3D.hpp"
#include "ComputeShader.hpp"
#include "Shader.hpp"
#include <array>
#include <vector>
#include <glm/glm.hpp>

namespace Renderer3D
{
	struct PostProcessVertex
	{
		glm::vec2 pos;
		glm::vec2 uv;
	};

	DXDevice* Device; DXDeviceContext* DeviceContext;

	// post processing
	Shader* postProcessShader;

	DXBuffer* postVertexBuffer, * postIndexBuffer;
	DXInputLayout* vertexLayout;
	RenderTexture* postRenderTexture;
	// post end
	void WindowScaleEvent(const int& x, const int& y);
}

RenderTexture* Renderer3D::GetPostRenderTexture() { return postRenderTexture; }


__forceinline static int FormatToByteSize(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_R32G32B32A32_FLOAT: return 16;
	case DXGI_FORMAT_R32G32B32_FLOAT: return 12;
	case DXGI_FORMAT_R32G32_FLOAT: return 8;
	case DXGI_FORMAT_R32_FLOAT: return 4;
	case DXGI_FORMAT_R32_SINT: return 4;
	default: return -1;
	}
}

DXInputLayout* Renderer3D::CreateVertexInputLayout(std::vector<InputLayoutCreateInfo> infos, DXBlob* VS_Buffer)
{
	DXInputLayout* vertLayout;

	D3D11_INPUT_ELEMENT_DESC* layout = (D3D11_INPUT_ELEMENT_DESC*)malloc(infos.size() * sizeof(D3D11_INPUT_ELEMENT_DESC));
	unsigned int byteIndex = 0;
	for (int i = 0; i < infos.size(); ++i)
	{
		layout[i] = { infos[i].name, 0, infos[i].format, 0, byteIndex, D3D11_INPUT_PER_VERTEX_DATA, 0 };
		byteIndex += FormatToByteSize(infos[i].format);
	}
	auto device = Engine::GetDevice();
	device->CreateInputLayout(layout, infos.size(), VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), &vertLayout);
	return vertLayout;
}

void Renderer3D::Initialize(DXDevice* _device, DXDeviceContext* _deviceContext, unsigned int msaaSamples)
{
	Device = _device; DeviceContext = _deviceContext;
	postProcessShader = new Shader("Shaders/PostProcessing.hlsl\0");

	std::array<PostProcessVertex, 4> vertices;
	vertices[0] = { glm::vec2(-1.0f, -1.0f), glm::vec2(1.0f, 1.0f) };
	vertices[1] = { glm::vec2(-1.0f,  1.0f), glm::vec2(1.0f, 0.0f) };
	vertices[2] = { glm::vec2(1.0f, -1.0f), glm::vec2(0.0f, 1.0f) };
	vertices[3] = { glm::vec2(1.0f,  1.0f), glm::vec2(0.0f, 0.0f) };

	std::array<uint16_t, 6> indices =
	{
		0,1,2, 1,3,2
	};

	CSCreateVertexIndexBuffers<PostProcessVertex, uint16_t>(
		Device, vertices.data(), indices.data(),
		vertices.size(), indices.size(), &postVertexBuffer, &postIndexBuffer);

	std::vector<InputLayoutCreateInfo> vertexLayoutInfos =
	{
		{"POSITION", DXGI_FORMAT_R32G32_FLOAT}, {"TEXCOORD", DXGI_FORMAT_R32G32_FLOAT}
	};
	vertexLayout = CreateVertexInputLayout(vertexLayoutInfos, postProcessShader->VS_Buffer);

	postRenderTexture = new RenderTexture(1000, 800, msaaSamples);
	Engine::AddWindowScaleEvent(WindowScaleEvent);
}

void Renderer3D::WindowScaleEvent(const int& width, const int& height)
{
	postRenderTexture->Invalidate(width, height);
}

void Renderer3D::PostProcessing(DXShaderResourceView* srv, DXTexSampler* sampler, bool build)
{
	if(!build) 
	postRenderTexture->SetAsRendererTarget();
	postProcessShader->Bind();

	DrawIndexedInfo drawInfo
	{
		DeviceContext, vertexLayout,
		postVertexBuffer, postIndexBuffer, 6
	};

	DeviceContext->PSSetShaderResources(0, 1, &srv);
	DeviceContext->PSSetSamplers(0, 1, &sampler);

	DrawIndexed16<PostProcessVertex>(&drawInfo);
}

void Renderer3D::Dispose()
{
	postProcessShader->Dispose();
}
