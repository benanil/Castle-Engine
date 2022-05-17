#pragma once
#include "../Rendering.hpp"
#include "Texture.hpp"


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

	std::string name;
		
	union  {
		Texture* textures[3];
		struct { Texture* albedo, * specular, * normal; };
	};

	Material();
	Material(const std::string _name);

	void CreateMaterialBuffer();

	void Serialize(const std::string& path);
	// returns false if version is not same
	void Deserialize(const std::string& path);

	void Bind();

#ifndef NEDITOR
	void OnEditor();
#endif

};