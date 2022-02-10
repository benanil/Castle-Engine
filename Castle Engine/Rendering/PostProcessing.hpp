#pragma once
#include "Renderer3D.hpp"
#include "Shader.hpp"
#include <array>

class PostProcessing
{
private:
	__declspec(align(16)) struct PostCbuffer
	{
		int width, height, mode;
		union {
			struct { float saturation; };
			struct { float treshold;   };
		};
	};

public:
	~PostProcessing();
	PostProcessing(ID3D11Device* Device, ID3D11DeviceContext* DeviceContext, unsigned int _msaaSamples);
	void Proceed(DXShaderResourceView* srv, DXTexSampler* sampler, bool build);
	void WindowScaleEvent(const int& _width, const int& _height);
	void OnEditor();
	void Dispose();
	Shader* GetShader() const { return postProcessShader; };
	RenderTexture* GetPostRenderTexture() const { return postRenderTexture; };
private:
	void RenderToQuad(RenderTexture* renderTexture, Shader* shader, DXShaderResourceView* srv, DXTexSampler* sampler, int scale);
	void RenderToQuad(RenderTexture* renderTexture, Shader* shader, RenderTexture* beforeTexture, int scale);
	void RenderToQuadUpsample(RenderTexture* renderTexture, Shader* shader,
		RenderTexture* miniRenderTexture, RenderTexture* biggerRenderTexture, int scale);
	void PostModeChanged();
private:
	bool disposed;
	float treshold = 0.95f;

	DXDevice* Device; DXDeviceContext* DeviceContext;
	
	Shader* postProcessShader;
	RenderTexture* postRenderTexture;
	DXBuffer* ScreenSizeCB;

	PostCbuffer postCbuffer;
	std::vector<MeshRenderer*> meshRenderers;

	std::array<RenderTexture*, 6> downSampleRTS; // rts = render textures
	std::array<RenderTexture*, 6> upSampleRTS; // rts = render textures

	Shader* FragPrefilter13, * FragPrefilter4,
		  * FragDownsample13, * FragDownsample4,
		  * FragUpsampleTent, * FragUpsampleBox;
};

