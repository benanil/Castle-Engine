#pragma once
#include "Texture.hpp"
#include "Mesh.hpp"
#include <string>
#include "../CE_Common.hpp"
#include "Shader.hpp"

class Material;
class Mesh;

namespace AssetManager
{
	// <summary> if texture is not finded and wrap mode is not zero creates new texture </summary>
	bool TryGetTexture (Texture*& texture  , const std::string& path, int wrapMode = 1);
	bool TryGetTexture (Texture*& texture  , uint hash);

	bool TryGetMaterial(Material*& material, const std::string& path);
	bool TryGetMaterial(Material*& material, uint hash);

	bool TryGetMesh(Mesh*& mesh, uint hash);
	
	bool TryGetShader(Shader*& shader, uint hash);
	bool TryGetShader(Shader*& shader, const std::string& hash);

	Texture * AddTexture (Texture*  texture, uint hash);
	Mesh    * AddMesh    (Mesh*  texture, uint hash);
	Material* AddMaterial(Material* material, uint hash);
	Material* AddMaterial(Material* material, const std::string& name);

	Shader* AddShader(Material* shader, uint hash);
	Shader* AddShader(Material* shader, const std::string& name);

	void Destroy();
}