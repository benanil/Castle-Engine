#include "AssetManager.hpp"
#include <filesystem>
#include "Mesh.hpp"
#include <D3D11.h>

namespace AssetManager
{
	std::unordered_map<uint, Mesh*>     Meshes   ;
	std::unordered_map<uint, Texture*>  Textures ;
	std::unordered_map<uint, Material*> Materials;
	std::unordered_map<uint, Shader*>   Shaders  ;
	
	bool TryGetTexture (Texture*& texture, const std::string& path, int wrapMode)
	{
		const uint hash = CSCore::StringToHash(path.c_str());
		if (Textures.find(hash) != Textures.end()) {
			texture = Textures[hash];
			return true;
		}
		texture = new Texture(path, (D3D11_TEXTURE_ADDRESS_MODE)wrapMode);
		return AddTexture(texture, hash) != nullptr;
	}

	bool TryGetTexture (Texture*& texture, uint hash)
	{
		if (Textures.find(hash) == Textures.end()) return false;
		texture = Textures[hash];
		return true;
	}

	bool TryGetMaterial(Material*& material , uint hash)
	{
		if (Materials.find(hash) == Materials.end()) return false;
		material = Materials[hash];
		return true;
	}

	bool TryGetMaterial(Material*& material, const std::string& path)
	{
		uint hash = CSCore::StringToHash(path.c_str());
		// try to find in chace
		if (Materials.find(hash) != Materials.end())	{
			material = Materials[hash];
			return true;
		}
		// if file also not exist return default material
		if (!std::filesystem::exists(path)) {
			material = MeshLoader::GetDefaultMaterial();
			std::cout << path << std::endl; // returns fabric_c ve need to convert this to data/sponzafabri
			std::cout << "material file is not exist returning default material" << std::endl;
			return false;
		}

		material = new Material();
		material->Deserialize(path);
		return true;
	}

	bool TryGetShader(Shader*& shader, uint hash)
	{
		if (Shaders.find(hash) == Shaders.end()) return false;
		shader = Shaders[hash];
		return true;
	}

	bool TryGetShader(Shader*& shader, const std::string& path)
	{
		uint hash = CSCore::StringToHash(path.c_str());
		return TryGetShader(shader, hash);
	}

	bool TryGetMesh(Mesh*& mesh, uint hash)
	{
		if (Meshes.find(hash) == Meshes.end()) return false;
		mesh = Meshes[hash];
		return true;
	}

	Texture* AddTexture (Texture*  texture, uint hash)
	{
 		Textures[hash] = texture;
		return texture;
	}

	Mesh* AddMesh (Mesh*  mesh, uint hash) {
		Meshes[hash] = mesh;
		return mesh;
	}
	
	Material* AddMaterial(Material* material, uint hash) {
		Materials[hash] = material;
		return material;
	}

	Material* AddMaterial(Material* material, const std::string& name) {
		Materials[CSCore::StringToHash(name.c_str())] = material;
		return material;
	}

	Shader* AddShader(Shader* shader, uint hash) {
		Shaders[hash] = shader;
		return shader;
	}

	Shader* AddShader(Shader* shader, const std::string& name)
	{ 
		Shaders[CSCore::StringToHash(name.c_str())] = shader;
		return shader;
	}

	// todo finish
	void Destroy() { }
}
