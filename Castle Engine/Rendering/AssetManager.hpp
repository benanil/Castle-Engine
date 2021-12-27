#pragma once
#include "Texture.hpp"
#include "Mesh.hpp"
#include <vector>

class Material;
namespace AssetManager
{
	Texture*  GetTexture(const char* path);
	Material* GetMaterial(const char* name);

	bool GetTextureP (const char* path, Texture**  texture);
	bool GetMaterialP(const char* name, Material** material);

	Texture*  AddTexture (Texture*  texture);
	Material* AddMaterial(Material* material);
}