#include "Mesh.hpp"
#include "spdlog/spdlog.h"

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
	Texture* whiteTexture, * blackTexture, * flatNormalTexture;

	Texture* ImportTexture(aiMaterial* const& aiMaterial, aiTextureType textureType, Texture* defaultTexture)
	{
		aiString aiPath;
		aiTextureMapMode aiWrapmode = (aiTextureMapMode)0;

		if (aiMaterial->GetTexture(textureType, 0, &aiPath, nullptr, nullptr, nullptr, nullptr, &aiWrapmode) == 0)
		{
			char cPath[] = "Models\\\0                                         ";
			const char* cResult = strcat(cPath, aiPath.C_Str());
			return AssetManager::AddTexture(new Texture(cResult, AssimpToD3D11_Wrap(aiWrapmode)));
		}
		else
		{
			return defaultTexture;
		}
	}

	MeshRenderer* LoadMesh(const std::string& path)
	{
		if (!std::filesystem::exists(path))
		{
			std::cout << path;
			assert(0, "mesh file is not exist");
		}

		if (firstImport)
		{
			whiteTexture = new Texture("Textures/dark_texture.png");
			blackTexture = new Texture("Textures/default texture.png");
			flatNormalTexture = new Texture("Textures/flat_normal_texture.png");
			firstImport = false;
		}

		MeshRenderer* mesh = new MeshRenderer();

		Assimp::Importer importer;
		static const uint32_t flags = aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded;
		const aiScene* scene = importer.ReadFile(path, flags);
		
		if (!scene)
		{
			std::cout << importer.GetErrorString() << std::endl;
			DX_CHECK(-1, (path + std::string("MESH LOADING FAILED!")).c_str());
			return nullptr;
		}

		mesh->subMeshes = (SubMesh*)malloc(sizeof(SubMesh) * scene->mNumMeshes);
		mesh->subMeshCount = scene->mNumMeshes;
		mesh->materials.resize(scene->mNumMaterials);

		for (uint16_t i = 0; i < scene->mNumMaterials; i++)
		{
			aiMaterial* const& _aiMaterial = scene->mMaterials[i];
			mesh->materials[i] = new Material();

			mesh->materials[i]->name = _aiMaterial->GetName().C_Str();
			mesh->materials[i]->albedo = ImportTexture(_aiMaterial, aiTextureType::aiTextureType_DIFFUSE, whiteTexture);
			mesh->materials[i]->specular = ImportTexture(_aiMaterial, aiTextureType::aiTextureType_SPECULAR, blackTexture);
			mesh->materials[i]->normal = ImportTexture(_aiMaterial, aiTextureType::aiTextureType_NORMALS, flatNormalTexture);

			for (uint8_t j = 0; j < _aiMaterial->mNumProperties; j++)
			{
				if (strcmp(_aiMaterial->mProperties[j]->mKey.C_Str(), "$clr.specular") == 0)
					mesh->materials[i]->cbuffer.shininesss = (*reinterpret_cast <float*> (_aiMaterial->mProperties[j]->mData)) / 8;
				else if (strcmp(_aiMaterial->mProperties[j]->mKey.C_Str(), "$mat.shininess") == 0)
					mesh->materials[i]->cbuffer.shininesss = (*reinterpret_cast <float*> (_aiMaterial->mProperties[j]->mData)) / 8;
			}
		}

		bool isSponza = strcmp(scene->mRootNode->mName.C_Str(), "sponza.obj") == 0;

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
			mesh->subMeshes[i] = SubMesh(*scene->mMeshes[i], isSponza);
			mesh->subMeshes[i].materialIndex = (uint16_t)scene->mMeshes[i]->mMaterialIndex;
		}
		return mesh;
	}
}