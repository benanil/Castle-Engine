#pragma once
#include "Renderer3D.hpp"
#include "ComputeShader.hpp"
#include "Shader.hpp"
#include <array>
#include <vector>
#include <glm/glm.hpp>
#ifndef NEDITOR
#	include "../Editor/Editor.hpp"
#endif

namespace Renderer3D
{
	struct PostProcessVertex
	{
		glm::vec2 pos;
		glm::vec2 uv;
	};

	__declspec(align(16)) struct PostCbuffer
	{
		int width, height, mode; float saturation;
	};

	DXDevice* Device; DXDeviceContext* DeviceContext;

	// post processing
	Shader* postProcessShader;

	DXBuffer* postVertexBuffer, * postIndexBuffer;
	DXInputLayout* vertexLayout;
	RenderTexture* postRenderTexture;
	DXBuffer* ScreenSizeCB;
	
	// post end
	PostCbuffer postCbuffer;
	std::vector<MeshRenderer*> meshRenderers;
	
	std::array<RenderTexture*, 6> downSampleRTS; // rts = render textures
	std::array<RenderTexture*, 6> upSampleRTS; // rts = render textures

	Shader* FragPrefilter13  ;
	Shader* FragPrefilter4   ;
	Shader* FragDownsample13 ;
	Shader* FragDownsample4  ;
	Shader* FragUpsampleTent ;
	Shader* FragUpsampleBox  ; 

	DrawIndexedInfo drawInfo;

	unsigned int MSAASamples;

	float treshold;

	void PostModeChanged();
	void RenderToQuad(RenderTexture* rendertTexture, Shader* shader,
		RenderTexture* srcRendertTexture, int scale);

	void RenderToQuad(
		RenderTexture* rendertTexture, Shader* shader,
		DXShaderResourceView* srv, DXTexSampler* sampler, int scale);

	void RenderToQuadUpsample(RenderTexture* rendertTexture, Shader* shader,
		RenderTexture* miniRenderTexture, RenderTexture* biggerRenderTexture1, int scale);

}

RenderTexture* Renderer3D::GetPostRenderTexture() { return postRenderTexture; }
const std::array<RenderTexture*, 6>& Renderer3D::GetDownsampleSRV() { return downSampleRTS; }
const std::array<RenderTexture*, 6>& Renderer3D::GetUpsampleSRV() { return upSampleRTS;  }


__forceinline static int FormatToByteSize(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_R32G32B32A32_FLOAT: return 16;
	case DXGI_FORMAT_R32G32B32_FLOAT: return 12;
	case DXGI_FORMAT_R32G32_FLOAT: return 8;
	case DXGI_FORMAT_R32G32_SINT: return 8;
	case DXGI_FORMAT_R32_FLOAT: return 4;
	case DXGI_FORMAT_R32_SINT: return 4;
	default: return -1;
	}
}

void Renderer3D::AddMeshRenderer(MeshRenderer* meshRenderer)
{
	meshRenderers.push_back(meshRenderer);
}

void Renderer3D::RenderMeshes()
{
	for (auto& renderer : meshRenderers)
	{
		renderer->Draw(DeviceContext);
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
	delete layout;
	return vertLayout;
}

void Renderer3D::Initialize(DXDevice* _device, DXDeviceContext* _deviceContext, unsigned int _msaaSamples)
{
	Device = _device; DeviceContext = _deviceContext;
	postProcessShader = new Shader("Shaders/PostProcessing.hlsl\0");

	FragPrefilter13  = new Shader("Shaders/DownSample.hlsl", "Shaders/DownSample.hlsl", "VS", "FragPrefilter13");
	FragPrefilter4   = new Shader("Shaders/DownSample.hlsl", "Shaders/DownSample.hlsl", "VS", "FragPrefilter4");
	FragDownsample13 = new Shader("Shaders/DownSample.hlsl", "Shaders/DownSample.hlsl", "VS", "FragDownsample13");
	FragDownsample4  = new Shader("Shaders/DownSample.hlsl", "Shaders/DownSample.hlsl", "VS", "FragDownsample4");
	FragUpsampleTent = new Shader("Shaders/DownSample.hlsl", "Shaders/DownSample.hlsl", "VS", "FragUpsampleTent");
	FragUpsampleBox  = new Shader("Shaders/DownSample.hlsl", "Shaders/DownSample.hlsl", "VS", "FragUpsampleBox");

	postRenderTexture = new RenderTexture(1000, 800, MSAASamples, RenderTextureCreateFlags::None, DXGI_FORMAT_R8G8B8A8_UNORM);

	for (int i = 0; i < downSampleRTS.size(); i++)
	{
		downSampleRTS[i] = new RenderTexture(1000 / (1 << i), 800 / (1 << i), MSAASamples, RenderTextureCreateFlags::Linear);
		upSampleRTS[i] = new RenderTexture(1000 / (1 << i), 800 / (1 << i), MSAASamples, RenderTextureCreateFlags::Linear);
	}

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

	// create screen size buffer
	DX_CREATE(D3D11_BUFFER_DESC, cbDesc);
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.ByteWidth = sizeof(PostCbuffer);
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	DX_CREATE(D3D11_SUBRESOURCE_DATA, vinitData);
	postCbuffer = {1000, 800, 0, 1.2f};
	vinitData.pSysMem = &postCbuffer;

	Device->CreateBuffer(&cbDesc, &vinitData, &ScreenSizeCB);

	drawInfo =
	{
		DeviceContext, vertexLayout,
		postVertexBuffer, postIndexBuffer, 6
	};
}

void Renderer3D::WindowScaleEvent(const int& _width, const int& _height)
{
	postCbuffer.width = _width; postCbuffer.height = _height;
	DeviceContext->UpdateSubresource(ScreenSizeCB, 0, NULL, &postCbuffer, 0, 0);
	postRenderTexture->Invalidate(postCbuffer.width, postCbuffer.height);
	
	for (int i = 0; i < downSampleRTS.size(); i++)
	{
		downSampleRTS[i]->Invalidate(_width / (1 << i), _height / (1 << i));
		upSampleRTS[i]->Invalidate(_width / (1 << i), _height / (1 << i));
	}
}


#ifndef NEDITOR
void Renderer3D::OnEditor()
{
	if (ImGui::DragFloat("Saturation", &postCbuffer.saturation, 0.001f)) PostModeChanged();
	ImGui::DragFloat("Treshold", &treshold, 0.01f);

	static std::array<const char*, 5> PostModes = { "ACES", "AMD", "Uncharted", "Reinhard", "DX11DSK" };

	Editor::GUI::EnumField(postCbuffer.mode, PostModes.data(), 5, "Modes", PostModeChanged);
}

#endif
void Renderer3D::PostModeChanged()
{
	DeviceContext->UpdateSubresource(ScreenSizeCB, 0, NULL, &postCbuffer, 0, 0);
}

void Renderer3D::RenderToQuad(RenderTexture* renderTexture, Shader* shader,
	RenderTexture* srcRenderTexture,  int scale) {
	RenderToQuad(renderTexture, shader, srcRenderTexture->textureView, srcRenderTexture->sampler, scale);
}

void Renderer3D::RenderToQuad(RenderTexture* rendertTexture, Shader* shader, DXShaderResourceView* srv, DXTexSampler* sampler,  int scale)
{
	rendertTexture->SetAsRendererTarget();
	shader->Bind();
	postCbuffer.mode = scale;
	DeviceContext->UpdateSubresource(ScreenSizeCB, 0, NULL, &postCbuffer, 0, 0);
	DeviceContext->PSSetConstantBuffers(0, 1, &ScreenSizeCB);
	DeviceContext->PSSetShaderResources(0, 1, &srv);
	DeviceContext->PSSetSamplers(0, 1, &sampler);

	DrawIndexed16<PostProcessVertex>(&drawInfo);
}

void Renderer3D::RenderToQuadUpsample(RenderTexture* rendertTexture, Shader* shader, 
	RenderTexture* miniRenderTexture, RenderTexture* biggerRenderTexture, int scale)
{
	rendertTexture->SetAsRendererTarget();
	shader->Bind();
	postCbuffer.mode = scale;

	std::vector<DXShaderResourceView*> SRVs = { miniRenderTexture->textureView, biggerRenderTexture->textureView };
	std::vector<DXTexSampler*> samplers     = { miniRenderTexture->sampler, biggerRenderTexture->sampler };

	DeviceContext->UpdateSubresource(ScreenSizeCB, 0, NULL, &postCbuffer, 0, 0);
	DeviceContext->PSSetConstantBuffers(0, 1, &ScreenSizeCB);
	DeviceContext->PSSetShaderResources(0, 2, SRVs.data());
	DeviceContext->PSSetSamplers(0, 2, samplers.data());

	DrawIndexed16<PostProcessVertex>(&drawInfo);
}

void Renderer3D::PostProcessing(DXShaderResourceView* srv, DXTexSampler* sampler, bool build)
{
	int startMode = postCbuffer.mode;
	float startSaturation = postCbuffer.saturation;
	postCbuffer.saturation = treshold;
	RenderToQuad(downSampleRTS[0], FragPrefilter13 , srv, sampler, 1 );
	RenderToQuad(downSampleRTS[1], FragPrefilter4  , downSampleRTS[0], 1 << 1);
	RenderToQuad(downSampleRTS[2], FragDownsample13, downSampleRTS[1], 1 << 2);
	RenderToQuad(downSampleRTS[3], FragDownsample4 , downSampleRTS[2], 1 << 3);
	RenderToQuad(downSampleRTS[4], FragDownsample4 , downSampleRTS[3], 1 << 4);
	RenderToQuad(downSampleRTS[5], FragDownsample4 , downSampleRTS[4], 1 << 5);
	
	RenderToQuadUpsample(upSampleRTS[4], FragUpsampleTent, downSampleRTS[5], downSampleRTS[4], 1 << 4);
	RenderToQuadUpsample(upSampleRTS[3], FragUpsampleBox , upSampleRTS[4]  , downSampleRTS[3], 1 << 3);
	RenderToQuadUpsample(upSampleRTS[2], FragUpsampleTent, upSampleRTS[3]  , downSampleRTS[2], 1 << 2);
	RenderToQuadUpsample(upSampleRTS[1], FragUpsampleBox , upSampleRTS[2]  , downSampleRTS[1], 1 << 1);
	RenderToQuadUpsample(upSampleRTS[0], FragUpsampleTent, upSampleRTS[1]  , downSampleRTS[0], 1 << 0);

	// todo combine here

	if(!build)	
	postRenderTexture->SetAsRendererTarget();
	postProcessShader->Bind();
	
	postCbuffer.mode = startMode;
	postCbuffer.saturation = startSaturation;
	DeviceContext->UpdateSubresource(ScreenSizeCB, 0, NULL, &postCbuffer, 0, 0);

	std::vector<DXShaderResourceView*> SRVs = { srv , upSampleRTS[0]->textureView };
	std::vector<DXTexSampler*> samplers = { sampler, upSampleRTS[0]->sampler};

	DeviceContext->PSSetShaderResources(0, 2, SRVs.data());
	DeviceContext->PSSetSamplers(0, 2, samplers.data());

	DrawIndexed16<PostProcessVertex>(&drawInfo);
}

void Renderer3D::Dispose()
{
	postProcessShader->Dispose();
}
