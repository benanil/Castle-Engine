#include "Mesh.hpp"
#include "spdlog/spdlog.h"
#include <assimp/postprocess.h> 
#include "../CE_Common.hpp"
#include "AssetManager.hpp"
#include "../Helper.hpp" // for GetBackSlashIndex and GetExtensionIndex for file name

namespace MeshLoader
{
	[[nodiscard]] inline
	D3D11_TEXTURE_ADDRESS_MODE AssimpToD3D11_Wrap(const aiTextureMapMode& aimode)
	{
		switch (aimode)
		{
			case aiTextureMapMode_Wrap:   return D3D11_TEXTURE_ADDRESS_WRAP;
			case aiTextureMapMode_Clamp:  return D3D11_TEXTURE_ADDRESS_CLAMP;
			case aiTextureMapMode_Decal:  return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
			case aiTextureMapMode_Mirror: return D3D11_TEXTURE_ADDRESS_MIRROR;
			default: D3D11_TEXTURE_ADDRESS_WRAP;
		}
	}

	bool firstImport = true;
	Texture* whiteTexture, * blackTexture, * flatNormalTexture, *missingTexture;
	Material* defaultMaterial;

	Texture* GetWhiteTexture()      { return whiteTexture;      }
	Texture* GetBlackTexture()      { return blackTexture;      } 
	Texture* GetFlatNormalTexture() { return flatNormalTexture; } 
	Texture* GetMissingTexture()    { return missingTexture;    }
	Material* GetDefaultMaterial()  { return defaultMaterial;   }

	Texture* ImportTexture(aiMaterial* const& aiMaterial, aiTextureType textureType, Texture* defaultTexture)
	{
		aiString aiPath;
		aiTextureMapMode aiWrapmode = (aiTextureMapMode)0;

		if (aiMaterial->GetTexture(textureType, 0, &aiPath, nullptr, nullptr, nullptr, nullptr, &aiWrapmode) == 0)
		{
			Texture* texture = nullptr;
			// fixme: maybe texture is in somewhere else we may change this line in future
			std::string path = "Models\\" + std::string(aiPath.C_Str());
			AssetManager::TryGetTexture(texture, path, AssimpToD3D11_Wrap(aiWrapmode));
			return texture;
		}
		else {
			return defaultTexture;
		}
	}

	Mesh* LoadCmesh(const std::string& path) {
		Mesh* mesh = new Mesh();
		// check version control
		mesh->LoadMesh(path);
		uint hash = CSCore::StringToHash(path.c_str());
		AssetManager::AddMesh(mesh, hash);
		return mesh;
	}

	Mesh* LoadMesh(const std::string& path)
	{
		if (!std::filesystem::exists(path)) {
			std::cout << path << std::endl;
			throw std::exception("mesh file is not exist");
		}

		// this is lazy ass implementation we could make this in constructor or in initialize function or something
		if (firstImport) {
			// get or add textures
			AssetManager::TryGetTexture(whiteTexture     , "Textures/dark_texture.png");
			AssetManager::TryGetTexture(blackTexture     , "Textures/default texture.png");
			AssetManager::TryGetTexture(flatNormalTexture, "Textures/flat_normal_texture.png");
			AssetManager::TryGetTexture(missingTexture   , "Textures/Missing.png");
			defaultMaterial = new Material();
			defaultMaterial->textures[0] = whiteTexture;
			defaultMaterial->textures[1] = whiteTexture;
			defaultMaterial->textures[2] = whiteTexture;
			firstImport = false;
		}

		Mesh* mesh = nullptr;

		int backSlashIndex = GetLastBackSlashIndex(path); // helper.hpp
		int extensionIndex = GetExtensionIndex(path); // helper.hpp

		// asd/bla.obj 7 - 4 = 4
		std::string fileName  = path.substr(backSlashIndex, extensionIndex - backSlashIndex);
		std::string extension = path.substr(extensionIndex);
		std::string directory = path.substr(0, backSlashIndex);

		uint hash = CSCore::StringToHash(fileName.c_str());
		
		std::cout << "directory: " << directory << std::endl;
		std::cout << "file name: " << fileName  << std::endl;
		std::cout << "extension: " << extension << std::endl;

		if (AssetManager::TryGetMesh(mesh, hash)) {
			std::cout << "mesh already imported using chace" << std::endl;
			return mesh;
		}
		
		// if mesh that we trying to import is cmesh (our custom binary mesh) load from disk not from assimp
		if (extension == ".cmesh")  return LoadCmesh(path);
		// check if filename+cmesh is exist 
		std::string newPath = directory + fileName + ".cmesh";
		if (std::filesystem::exists(newPath)) return LoadCmesh(newPath);

		Assimp::Importer importer;
		constexpr uint32_t flags = aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded;
		const aiScene* scene = importer.ReadFile(path, flags);

		mesh = new Mesh( fileName );
		
		if (!scene) {
			std::cout << importer.GetErrorString() << fileName << std::endl;
			throw std::exception("Assimp Mesh Loading Failed!");
		}

		mesh->subMeshes.reserve(scene->mNumMeshes);
		mesh->subMeshCount = scene->mNumMeshes;
		mesh->materials.reserve(scene->mNumMaterials);

		for (uint16_t i = 0; i < scene->mNumMaterials; i++)
		{
			aiMaterial* const& _aiMaterial = scene->mMaterials[i];
			Material* material = new Material();

			material->name 	   = std::string((const char*)_aiMaterial->GetName().C_Str());
			material->albedo   = ImportTexture(_aiMaterial, aiTextureType::aiTextureType_DIFFUSE, whiteTexture);
			material->specular = ImportTexture(_aiMaterial, aiTextureType::aiTextureType_SPECULAR, blackTexture);
			material->normal   = ImportTexture(_aiMaterial, aiTextureType::aiTextureType_NORMALS, flatNormalTexture);

			for (uint8_t j = 0; j < _aiMaterial->mNumProperties; j++)
			{
				if (strcmp(_aiMaterial->mProperties[j]->mKey.C_Str(), "$clr.specular") == 0
					or strcmp(_aiMaterial->mProperties[j]->mKey.C_Str(), "$mat.shininess") == 0) {
					material->cbuffer.shininesss = (*reinterpret_cast <float*> (_aiMaterial->mProperties[j]->mData)) / 8;
					break;
				}
			}
			mesh->materials.push_back(material);
			// test
			std::string matPath = "Data\\" + fileName + material->name + ".mat";
			material->Serialize  (matPath);
			material->Deserialize(matPath);
			AssetManager::AddMaterial(mesh->materials[i], mesh->materials[i]->name);
		}

		bool isSponza = extension == ".obj"; // is obj

		if (scene->mNumMaterials == 0)
		{
			mesh->materials.resize(1);
			mesh->materials[0] = new Material();
			mesh->materials[0]->albedo = whiteTexture;
			mesh->materials[0]->specular = blackTexture;
			mesh->materials[0]->normal = flatNormalTexture;
		}

		for (uint16_t i = 0; i < scene->mNumMeshes; i++)
		{
			mesh->subMeshes.push_back(SubMesh(*scene->mMeshes[i], isSponza));
			mesh->subMeshes.back().materialIndex = (uint16_t)scene->mMeshes[i]->mMaterialIndex;
		}
		mesh->subMeshCount = (uint32_t)mesh->subMeshes.size();
		// save mesh as binary
		mesh->SaveMesh(directory);
		// mesh->LoadMesh(directory);
		// store mesh pointer in unordered_map so we don't need to load sponza multiple times
		AssetManager::AddMesh(mesh, hash);
		return mesh;
	}
}