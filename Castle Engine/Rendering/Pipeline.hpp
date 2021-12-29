#pragma once
#include "../Helper.hpp"
#include "Shader.hpp"
#include "RenderTexture.hpp"
#include "../Engine.hpp"

class Pipeline
{
public:
	Shader* shader;
	RenderTexture* renderTexture;
private:
	DXDeviceContext* context;
	DXBuffer* constantBuffers;
	DXInputLayout* vertLayout;
	DXBlendState* blendState;
public:
	Pipeline(
		Shader* _shader, 
		const int& width, 
		const int& height,
		DXInputLayout* _vertLayout = nullptr) 
	:  shader(_shader), vertLayout(_vertLayout)
	{
		context = Engine::GetDeviceContext();
		renderTexture = new RenderTexture(width, height, 1);
	}

	void Bind()
	{
		renderTexture->SetAsRendererTarget();
		context->IASetInputLayout(vertLayout);
	}
};

