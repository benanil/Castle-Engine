#include "PostProcessing.hpp"
#include "../Rendering.hpp"
#include "../Editor/Editor.hpp"

PostProcessing::~PostProcessing() { Dispose(); }

PostProcessing::PostProcessing(ID3D11Device* _Device, ID3D11DeviceContext* _DeviceContext, unsigned int MSAASamples)
	: Device(_Device), DeviceContext(_DeviceContext)
{
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

	// create screen size buffer
	DX_CREATE(D3D11_BUFFER_DESC, cbDesc);
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.ByteWidth = sizeof(PostCbuffer);
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	DX_CREATE(D3D11_SUBRESOURCE_DATA, vinitData);
	postCbuffer = {1000, 800, 0, 1.2f};
	vinitData.pSysMem = &postCbuffer;

	Device->CreateBuffer(&cbDesc, &vinitData, &ScreenSizeCB);
}

void PostProcessing::Proceed(DXShaderResourceView* srv, DXTexSampler* sampler, bool build)
{
	int startMode = postCbuffer.mode;
	float startSaturation = postCbuffer.saturation;
	postCbuffer.treshold = treshold;

	RenderToQuad(downSampleRTS[0], FragPrefilter13 , srv, sampler, 1 << 0);
	RenderToQuad(downSampleRTS[1], FragPrefilter4  , downSampleRTS[0], 1 << 1);
	RenderToQuad(downSampleRTS[2], FragDownsample13, downSampleRTS[1], 1 << 2);
	RenderToQuad(downSampleRTS[3], FragDownsample4 , downSampleRTS[2], 1 << 3);
	RenderToQuad(downSampleRTS[4], FragDownsample13, downSampleRTS[3], 1 << 4);
	RenderToQuad(downSampleRTS[5], FragDownsample4 , downSampleRTS[4], 1 << 5);
	
	RenderToQuadUpsample(upSampleRTS[4], FragUpsampleTent, downSampleRTS[5], downSampleRTS[4], 1 << 4);
	RenderToQuadUpsample(upSampleRTS[3], FragUpsampleBox , upSampleRTS[4]  , downSampleRTS[3], 1 << 3);
	RenderToQuadUpsample(upSampleRTS[2], FragUpsampleTent, upSampleRTS[3]  , downSampleRTS[2], 1 << 2);
	RenderToQuadUpsample(upSampleRTS[1], FragUpsampleBox , upSampleRTS[2]  , downSampleRTS[1], 1 << 1);
	RenderToQuadUpsample(upSampleRTS[0], FragUpsampleTent, upSampleRTS[1]  , downSampleRTS[0], 1 << 0);

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

	Renderer3D::RenderToQuad();
}

void PostProcessing::RenderToQuad(RenderTexture* renderTexture, Shader* shader, RenderTexture* beforeTexture, int scale)
{
	RenderToQuad(renderTexture, shader, beforeTexture->textureView, beforeTexture->sampler, scale);
}
void PostProcessing::RenderToQuad(RenderTexture* renderTexture, Shader* shader, DXShaderResourceView* srv, DXTexSampler* sampler,  int scale)
{
	shader->Bind();
	renderTexture->SetAsRendererTarget();

	postCbuffer.mode = scale;
	DeviceContext->UpdateSubresource(ScreenSizeCB, 0, NULL, &postCbuffer, 0, 0);
	DeviceContext->PSSetConstantBuffers(0, 1, &ScreenSizeCB);
	DeviceContext->PSSetShaderResources(0, 1, &srv);
	DeviceContext->PSSetSamplers(0, 1, &sampler);
	Renderer3D::RenderToQuad();
}

void PostProcessing::RenderToQuadUpsample(RenderTexture* renderTexture, Shader* shader, 
	RenderTexture* miniRenderTexture, RenderTexture* biggerRenderTexture, int scale)
{
	shader->Bind();
	renderTexture->SetAsRendererTarget();

	postCbuffer.mode = scale;

	std::vector<DXShaderResourceView*> SRVs = { miniRenderTexture->textureView, biggerRenderTexture->textureView };
	std::vector<DXTexSampler*> samplers     = { miniRenderTexture->sampler, biggerRenderTexture->sampler };

	DeviceContext->UpdateSubresource(ScreenSizeCB, 0, NULL, &postCbuffer, 0, 0);
	DeviceContext->PSSetConstantBuffers(0, 1, &ScreenSizeCB);
	DeviceContext->PSSetShaderResources(0, 2, SRVs.data());
	DeviceContext->PSSetSamplers(0, 2, samplers.data());

	Renderer3D::RenderToQuad();
}

void PostProcessing::PostModeChanged()
{
	DeviceContext->UpdateSubresource(ScreenSizeCB, 0, NULL, &postCbuffer, 0, 0); 
}

void PostProcessing::WindowScaleEvent(const int& _width, const int& _height)
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

void PostProcessing::OnEditor()
{
#ifndef NEDITOR
	if (ImGui::CollapsingHeader("PostProcessing"))
	{
		static std::array<const char*, 5> PostModes = { "ACES", "AMD", "Uncharted", "Reinhard", "DX11DSK" };

		if (Editor::GUI::EnumField(postCbuffer.mode, PostModes.data(), 5, "Modes")) PostModeChanged();
		if (ImGui::DragFloat("Saturation", &postCbuffer.saturation, 0.001f)) PostModeChanged();

		ImGui::Text("Bloom");
		ImGui::DragFloat("Treshold", &treshold, 0.01f);
	}
#endif // !NEDITOR
}

void PostProcessing::Dispose()
{
	if (disposed) return;
	disposed = true;
	postRenderTexture->Release();
	postProcessShader->Dispose();
	FragPrefilter13 ->Dispose(); FragPrefilter4 ->Dispose();
	FragDownsample13->Dispose(); FragDownsample4->Dispose();
	FragUpsampleTent->Dispose(); FragUpsampleBox->Dispose();
}