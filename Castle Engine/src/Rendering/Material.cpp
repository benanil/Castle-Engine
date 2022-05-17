#include "Material.hpp"
#include "../DirectxBackend.hpp"
#include "AssetManager.hpp"
#include <fstream>

#ifndef NEDITOR
#	include "../Editor/Editor.hpp"
#endif

Material::Material() : name("material") { CreateMaterialBuffer(); }
Material::Material(const std::string _name) : name(std::move(_name)) { CreateMaterialBuffer(); }

void Material::CreateMaterialBuffer()
{
	DXDevice* device = DirectxBackend::GetDevice();

	DX_CREATE(D3D11_BUFFER_DESC, cbDesc);
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.ByteWidth = sizeof(MaterialCBuffer);
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = 0;
	cbDesc.MiscFlags = 0;

	device->CreateBuffer(&cbDesc, NULL, &materialCBuffer);
}

static inline std::string GetTextureDescStr(Texture* texture)
{
	return std::to_string((int)texture->wrapMode) + texture->path + "\n";
}

void Material::Serialize(const std::string& path)
{
	std::ofstream ofs = std::ofstream(path, std::ios::out | std::ios::binary);
	std::string currWrite = "Name: " + name + "\n";
	// write name 
	ofs.write(currWrite.c_str(), currWrite.size());
	// write shader
	currWrite = "Shader:   " + std::string("default") + "\n";
	ofs.write(currWrite.c_str(), currWrite.size());
	// write textures
	currWrite = "Albedo:   " + GetTextureDescStr(albedo);
	ofs.write(currWrite.c_str(), currWrite.size());
	currWrite = "Specular: " + GetTextureDescStr(specular);
	ofs.write(currWrite.c_str(), currWrite.size());
	currWrite = "Normal:   " + GetTextureDescStr(normal);
	ofs.write(currWrite.c_str(), currWrite.size());
	
	ofs.close();
}

void Material::Deserialize(const std::string& path)
{
	std::ifstream ifs = std::ifstream(path, std::ios::in | std::ios::binary);
	std::string line;
	std::getline(ifs, line);
	name = line.substr(6, line.size() - 6);
	// fixme! for now we have only one shader
	// get shader name 
	std::getline(ifs, line); // pass this line we will use this line later

	for (char i = 0; i < 3; ++i)
	{
		std::getline(ifs, line);
		std::string texPath = line.substr(11, line.size() - 10);
		D3D11_TEXTURE_ADDRESS_MODE wrapMode = (D3D11_TEXTURE_ADDRESS_MODE)(line[10] - '0');

		if (!std::filesystem::exists(texPath) || !AssetManager::TryGetTexture(textures[i], texPath, wrapMode)) {
			textures[i] = MeshLoader::GetMissingTexture();
			std::cout << "material texture is missing using missing texture instead!" << std::endl;
		}
	}

	ifs.close();
}

void Material::Bind()
{
	DXDeviceContext* context = DirectxBackend::GetDeviceContext();
	if (albedo) albedo->Bind(context, 0);
	if (specular) specular->Bind(context, 1);
	if (normal)   normal->Bind(context, 2);

	context->UpdateSubresource(materialCBuffer, 0, NULL, &cbuffer, 0, 0);
	context->PSSetConstantBuffers(1, 1, &materialCBuffer);
}

#ifndef NEDITOR
void Material::OnEditor()
{
	if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_Bullet))
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.8f);
		ImGui::PushStyleColor(ImGuiCol_Border, HEADER_COLOR);
		ImGui::Text(name.c_str());

		Editor::GUI::TextureField("albedo", albedo);
		Editor::GUI::TextureField("specular", specular);
		Editor::GUI::TextureField("normal", normal);

		ImGui::DragFloat("shininess", &cbuffer.shininesss, 0.1f);
		ImGui::DragFloat("roughness ", &cbuffer.roughness, 0.01f);
		ImGui::DragFloat("metallic ", &cbuffer.metallic, 0.01f);

		ImGui::PopStyleColor();
		ImGui::PopStyleVar();
	}
}
#endif
