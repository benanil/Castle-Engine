#include "AssetManager.hpp"

namespace AssetManager
{
	std::vector<Material*> materials;
	std::vector<Texture*> textures;

	Texture* GetTexture(const char* path)
	{
		for (auto& texture : textures)
		{
			if (strcmp(texture->path, path))
			{
				return texture;
			}
		}
		return nullptr;
	}

	Material* GetMaterial(const char* name)
	{
		for (auto& material : materials)
		{
			if (strcmp(material->name, name))
			{
				return material;
			}
		}
		return nullptr;
	}

	bool GetTextureP(const char* path, Texture**  texture)
	{
		*texture = GetTexture(path);
		return (*texture) != nullptr;
	}
	
	bool GetMaterialP(const char* name, Material** material)
	{
		*material = GetMaterial(name);
		return (*material) != nullptr;
	}

	Texture* AddTexture (Texture*  texture)
	{
		textures.push_back(texture);
		return texture;
	}
	
	Material* AddMaterial(Material* material)
	{
		materials.push_back(material);
		return material;
	}
}