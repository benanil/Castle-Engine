#pragma once
#include "../Helper.hpp"
#include "Texture.hpp"
#include "../Engine.hpp"
#ifndef NEDITOR
#	include "../Editor/Editor.hpp"
#endif

struct MaterialCBuffer
{
	float shininesss = 2.0f;
	float roughness  = 1.5f;
	float metallic   = 0.55f;
	float padding    = 0.0f;      
};

class Material
{
public:
	MaterialCBuffer cbuffer;
	DXBuffer* materialCBuffer;

	const char* name;
	Texture* albedo, *specular, *normal;

	Material() : name("material") { CreateMaterialBuffer(); }
	Material(const char* _name) : name(_name) { CreateMaterialBuffer(); }

	void CreateMaterialBuffer()
	{
		DXDevice* device = Engine::GetDevice();

		DX_CREATE(D3D11_BUFFER_DESC, cbDesc);
		cbDesc.Usage = D3D11_USAGE_DEFAULT;
		cbDesc.ByteWidth = sizeof(MaterialCBuffer);
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = 0;
		cbDesc.MiscFlags = 0;

		device->CreateBuffer(&cbDesc, NULL, &materialCBuffer);
	}

	void Bind()
	{
		DXDeviceContext* context = Engine::GetDeviceContext();
		if (albedo)   albedo->Bind(context, 0);
		if (specular) specular->Bind(context, 1);
		if (normal)   normal->Bind(context, 2);
		
		context->UpdateSubresource(materialCBuffer, 0, NULL, &cbuffer, 0, 0);
		context->PSSetConstantBuffers(1, 1, &materialCBuffer);
	}

#ifndef NEDITOR
	void OnEditor()
	{
		if (ImGui::CollapsingHeader("Material"))
		{
			Editor::GUI::TextureField("albedo", albedo->resourceView);
			Editor::GUI::TextureField("specular", specular->resourceView);
			Editor::GUI::TextureField("normal", normal->resourceView);

			ImGui::DragFloat("shininess", &cbuffer.shininesss, 0.1f);
			ImGui::DragFloat("roughness ", &cbuffer.roughness, 0.01f);
			ImGui::DragFloat("metallic ", &cbuffer.metallic, 0.01f);
		}
	}
#endif

};