#include "PostProcessing.hpp"
#include "../Rendering.hpp"
#include "../Editor/Editor.hpp"
#include "../Engine.hpp"
#include "../DirectxBackend.hpp"
#include "FXAA.hpp"
#include <GFSDK_SSAO.h>
#include "Shadow.hpp"

namespace PostProcessing
{
	__declspec(align(16)) struct PostCbuffer
	{
		int width, height, mode;
		union {
			struct { float saturation; };
			struct { float treshold; };
		};
		glm::vec2 sunPos;
	};

	void RenderToQuad(RenderTexture* renderTexture, Shader* shader, DXShaderResourceView* srv, DXTexSampler* sampler, int scale);
	void RenderToQuad(RenderTexture* renderTexture, Shader* shader, RenderTexture* beforeTexture, int scale);
	void RenderToQuadUpsample(RenderTexture* renderTexture, Shader* shader,
		RenderTexture* miniRenderTexture, RenderTexture* biggerRenderTexture, int scale);
	void PostModeChanged();
	bool disposed;

	DXDevice* Device; DXDeviceContext* DeviceContext;

	Shader* postProcessShader;
	RenderTexture* postRenderTexture;
	DXBuffer* ScreenSizeCB;

	PostCbuffer postCbuffer;
	std::vector<MeshRenderer*> meshRenderers;

	// bloom
	float treshold = 0.95f;
	std::array<RenderTexture*, 6> downSampleRTS; // rts = render textures
	std::array<RenderTexture*, 6> upSampleRTS; // rts = render textures

	Shader* FragPrefilter13, * FragPrefilter4,
		  * FragDownsample13, * FragDownsample4,
		  * FragUpsampleTent, * FragUpsampleBox;

	// ssao
	GFSDK_SSAO_Context_D3D11* pAOContext;
	RenderTexture* SSAORenderTexture;
	GFSDK_SSAO_Parameters SSAOParams {};
	float SSAOMetersToViewSpaceUnits = 1.0f;
	bool SSAOEnabled = true;
	// fxaa
	bool fxaaOpen = true;
}

Shader* PostProcessing::GetShader() { return postProcessShader; };
RenderTexture* PostProcessing::GetPostRenderTexture() { return postRenderTexture; };

void PostProcessing::Initialize(ID3D11Device* _Device, ID3D11DeviceContext* _DeviceContext, unsigned int MSAASamples)
{
	Device = _Device; DeviceContext = _DeviceContext;
	FXAA::Initialize();

	postProcessShader = new Shader("Shaders/PostProcessing.hlsl\0");
	FragPrefilter13   = new Shader("Shaders/Bloom.hlsl", "Shaders/Bloom.hlsl", "VS", "FragPrefilter13");
	FragPrefilter4    = new Shader("Shaders/Bloom.hlsl", "Shaders/Bloom.hlsl", "VS", "FragPrefilter4");
	FragDownsample13  = new Shader("Shaders/Bloom.hlsl", "Shaders/Bloom.hlsl", "VS", "FragDownsample13");
	FragDownsample4   = new Shader("Shaders/Bloom.hlsl", "Shaders/Bloom.hlsl", "VS", "FragDownsample4");
	FragUpsampleTent  = new Shader("Shaders/Bloom.hlsl", "Shaders/Bloom.hlsl", "VS", "FragUpsampleTent");
	FragUpsampleBox   = new Shader("Shaders/Bloom.hlsl", "Shaders/Bloom.hlsl", "VS", "FragUpsampleBox");

	glm::ivec2 WindowSize = Engine::GetMainMonitorScale();

	postRenderTexture = new RenderTexture(WindowSize.x, WindowSize.y, MSAASamples, RenderTextureCreateFlags::None, DXGI_FORMAT_R8G8B8A8_UNORM);

	for (int i = 0; i < downSampleRTS.size(); i++)
	{
		downSampleRTS[i] = new RenderTexture(WindowSize.x >> i, WindowSize.y >> i, MSAASamples, RenderTextureCreateFlags::Linear);
		upSampleRTS[i] = new RenderTexture(WindowSize.x >> i, WindowSize.y >> i, MSAASamples, RenderTextureCreateFlags::Linear);
	}

	// create screen size buffer
	DX_CREATE(D3D11_BUFFER_DESC, cbDesc);
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.ByteWidth = sizeof(PostCbuffer);
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	DX_CREATE(D3D11_SUBRESOURCE_DATA, vinitData);
	postCbuffer = { WindowSize.x, WindowSize.y, 0, 1.2f };
	vinitData.pSysMem = &postCbuffer;

	Device->CreateBuffer(&cbDesc, &vinitData, &ScreenSizeCB);
	// prepare ssao
	GFSDK_SSAO_CustomHeap CustomHeap;
	CustomHeap.new_ = ::operator new;
	CustomHeap.delete_ = ::operator delete;

	GFSDK_SSAO_Status status = GFSDK_SSAO_CreateContext_D3D11(Device, &pAOContext, &CustomHeap);
	assert(status == GFSDK_SSAO_OK); // HBAO+ requires feature level 11_0 or above
	SSAORenderTexture = new RenderTexture(WindowSize.x, WindowSize.y, MSAASamples,  RenderTextureCreateFlags::None, DXGI_FORMAT_R8G8B8A8_UNORM);

	SSAOParams.Radius = 1.f;
	SSAOParams.Bias = 0.2f;
	SSAOParams.PowerExponent = 1.5f;
	SSAOParams.Blur.Enable = true;
	SSAOParams.Blur.Radius = GFSDK_SSAO_BLUR_RADIUS_4;
	SSAOParams.Blur.SharpnessProfile.Enable = true;
	SSAOParams.Blur.Sharpness = 6.f;
}

void PostProcessing::Proceed(RenderTexture& rt, FreeCamera* camera)
{
	int startMode = postCbuffer.mode;
	float startSaturation = postCbuffer.saturation;
	postCbuffer.treshold = treshold;

	// proceed bloom
	RenderToQuad(downSampleRTS[0], FragPrefilter13 , rt.textureView, rt.sampler, 1 << 0);
	RenderToQuad(downSampleRTS[1], FragPrefilter4  , downSampleRTS[0], 1 << 1);
	RenderToQuad(downSampleRTS[2], FragDownsample13, downSampleRTS[1], 1 << 2);
	RenderToQuad(downSampleRTS[3], FragDownsample4 , downSampleRTS[2], 1 << 3);
	RenderToQuad(downSampleRTS[4], FragDownsample13, downSampleRTS[3], 1 << 4);
	RenderToQuad(downSampleRTS[5], FragDownsample4 , downSampleRTS[4], 1 << 5);

	RenderToQuadUpsample(upSampleRTS[4], FragUpsampleTent, downSampleRTS[5], downSampleRTS[4], 1 << 4);
	RenderToQuadUpsample(upSampleRTS[3], FragUpsampleBox , upSampleRTS[4], downSampleRTS[3], 1 << 3);
	RenderToQuadUpsample(upSampleRTS[2], FragUpsampleTent, upSampleRTS[3], downSampleRTS[2], 1 << 2);
	RenderToQuadUpsample(upSampleRTS[1], FragUpsampleBox , upSampleRTS[2], downSampleRTS[1], 1 << 1);
	RenderToQuadUpsample(upSampleRTS[0], FragUpsampleTent, upSampleRTS[1], downSampleRTS[0], 1 << 0);
	
	// proceed Nvidia HBAO+
	if (SSAOEnabled)
	{
		GFSDK_SSAO_InputData_D3D11 Input{};
		Input.DepthData.DepthTextureType = GFSDK_SSAO_HARDWARE_DEPTHS;
		Input.DepthData.pFullResDepthTextureSRV = rt.depthSRV;
		Input.DepthData.ProjectionMatrix.Data = *(const GFSDK_SSAO_Float4x4*)&camera->GetProjection();
		Input.DepthData.ProjectionMatrix.Layout = GFSDK_SSAO_ROW_MAJOR_ORDER;
		Input.DepthData.MetersToViewSpaceUnits = SSAOMetersToViewSpaceUnits;

		GFSDK_SSAO_Output_D3D11 Output;
		Output.pRenderTargetView = SSAORenderTexture->renderTargetView;
		Output.Blend.Mode = GFSDK_SSAO_OVERWRITE_RGB;

		GFSDK_SSAO_Status status = pAOContext->RenderAO(DeviceContext, Input, SSAOParams, Output);
		assert(status == GFSDK_SSAO_OK);
	}
	else
	{
		SSAORenderTexture->SetAsRendererTarget();
		static const float ClearColor[]{ 1.0f, 1.0f, 1.0f, 1.0f };
		SSAORenderTexture->ClearRenderTarget(ClearColor);
	}
	
#ifndef NEDITOR
	postRenderTexture->SetAsRendererTarget();
#else
	DirectxBackend::SetBackBufferAsRenderTarget();
#endif
	// render post processing
	postProcessShader->Bind();

	float sunAngle = Renderer3D::GetGlobalCbuffer().sunAngle;
	glm::vec2 clipCoords = camera->WorldToNDC(glm::vec3(0, -sin(sunAngle * DX_DEG_TO_RAD) * 10000.0f, -cos(sunAngle * DX_DEG_TO_RAD) * 10000.0f), XMMatrixIdentity());

	postCbuffer.mode = startMode;
	postCbuffer.saturation = startSaturation;
	postCbuffer.sunPos.x = (clipCoords.x + 1.0f) / 2.0f;
	postCbuffer.sunPos.y = 1.0f - ((clipCoords.y + 1.0f) / 2.0f);

	DeviceContext->UpdateSubresource(ScreenSizeCB, 0, NULL, &postCbuffer, 0, 0);

	std::array<DXShaderResourceView*, 4> SRVs = { rt.textureView , upSampleRTS[0]->textureView, SSAORenderTexture->textureView, rt.depthSRV };
	std::array<DXTexSampler*, 4> samplers     = { rt.sampler, upSampleRTS[0]->sampler, SSAORenderTexture->sampler, SSAORenderTexture->sampler };

	DeviceContext->PSSetShaderResources(0, SRVs.size(), SRVs.data());
	DeviceContext->PSSetSamplers(0, samplers.size(), samplers.data());

	Renderer3D::RenderToQuad();
	if (fxaaOpen) {
		FXAA::Proceed(*postRenderTexture);
	}
}

void PostProcessing::RenderToQuad(RenderTexture* renderTexture, Shader* shader, RenderTexture* beforeTexture, int scale)
{
	RenderToQuad(renderTexture, shader, beforeTexture->textureView, beforeTexture->sampler, scale);
}

void PostProcessing::RenderToQuad(RenderTexture* renderTexture, Shader* shader, DXShaderResourceView* srv, DXTexSampler* sampler, int scale)
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

	std::array<DXShaderResourceView*, 2> SRVs = { miniRenderTexture->textureView, biggerRenderTexture->textureView };
	std::array<DXTexSampler*, 2> samplers = { miniRenderTexture->sampler, biggerRenderTexture->sampler };

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
	SSAORenderTexture->Invalidate(postCbuffer.width, postCbuffer.height);

	for (int i = 0; i < downSampleRTS.size(); i++)
	{
		downSampleRTS[i]->Invalidate(_width >> i, _height >> i);
		upSampleRTS[i]->Invalidate(_width >> i, _height >> i);
	}
}

void PostProcessing::OnEditor()
{
#ifndef NEDITOR
	if (ImGui::CollapsingHeader("PostProcessing", ImGuiTreeNodeFlags_Bullet))
	{
		static std::array<const char*, 5> PostModes = { "ACES", "AMD", "Uncharted", "Reinhard", "DX11DSK" };
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.8f);
		ImGui::PushStyleColor(ImGuiCol_Border, HEADER_COLOR);
		if (Editor::GUI::EnumField(postCbuffer.mode, PostModes.data(), 5, "Modes")) PostModeChanged();
		if (ImGui::DragFloat("Saturation", &postCbuffer.saturation, 0.001f)) PostModeChanged();

		ImGui::Text("Bloom");
		ImGui::DragFloat("Treshold", &treshold, 0.01f);
		
		ImGui::Checkbox("Fxaa", &fxaaOpen);

		ImGui::PopStyleColor();
		ImGui::PopStyleVar();

		ImGui::Indent();

		if (ImGui::CollapsingHeader("SSAO", ImGuiTreeNodeFlags_Bullet))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.8f);
			ImGui::PushStyleColor(ImGuiCol_Border, HEADER_COLOR);

			ImGui::Checkbox("Enabled", &SSAOEnabled);
			ImGui::DragFloat("Radius", &SSAOParams.Radius, 0.05f);
			ImGui::DragFloat("Bias", &SSAOParams.Bias, 0.05f);
			ImGui::DragFloat("PowerExponent", &SSAOParams.PowerExponent, 0.05f, 0, 4);
			ImGui::DragFloat("BlurSharpness", &SSAOParams.Blur.Sharpness, 0.05f, 0, 8);
			ImGui::DragFloat("MetersToViewSpaceUnits", &SSAOMetersToViewSpaceUnits, 0.1f, 0, 10);
		
			ImGui::PopStyleColor();
			ImGui::PopStyleVar();
		}
		ImGui::Unindent();
	}

#endif // !NEDITOR
}

void PostProcessing::Dispose()
{
	if (disposed) return;
	disposed = true;
	postRenderTexture->Release();
	postProcessShader->Dispose();
	FragPrefilter13->Dispose(); FragPrefilter4->Dispose();
	FragDownsample13->Dispose(); FragDownsample4->Dispose();
	FragUpsampleTent->Dispose(); FragUpsampleBox->Dispose();
}