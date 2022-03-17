#ifndef NEDITOR

#include "Editor.hpp"
#include <spdlog/spdlog.h>
#include "../Rendering/Texture.hpp"

namespace Editor::ResourcesWindow
{
	std::filesystem::path currentPath;

	DXShaderResourceView* fileIcon   ;
	DXShaderResourceView* folderIcon ;
	DXShaderResourceView* meshIcon   ;
	DXShaderResourceView* fileBack   ;

	std::filesystem::path GetCurrentPath()
	{
		return currentPath;
	}

	void Initialize()
	{
		fileIcon   = Texture("Textures/file.png").resourceView;
		folderIcon = Texture("Textures/folder.png").resourceView;
		meshIcon   = Texture("Textures/mesh.png").resourceView;
		fileBack   = Texture("Textures/fileBack.png").resourceView;
		currentPath = std::filesystem::current_path();
	}

	void DrawWindow()
	{
		ImGui::Begin("Resources");

		if (ImGui::Button(ICON_FA_ARROW_LEFT))
		{
			currentPath = currentPath.parent_path();
		}

		// todo: make path static & add oppen close folders & back button & dragg and drop
		int id = 0; // for imgui push id

		ImGui::Separator();

#undef min

		for (auto& directory : std::filesystem::directory_iterator(currentPath))
		{
			std::string fileName = directory.path().filename().u8string();

			if (directory.is_directory()) {
				ImGui::PushID(id++);

				ImGui::BeginGroup();
				if (GUI::ImageButton(folderIcon))
				{
					spdlog::info("clicked {0}", fileName);
					currentPath = directory;
				}
				ImGui::TextWrapped(fileName.substr(0, std::min<int>(10, fileName.length())).c_str());
				const int width = ImGui::GetColumnWidth();

				ImGui::EndGroup();

				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::Text(fileName.c_str());
					ImGui::EndTooltip();
				}

				ImGui::PopID();

				if (width > filesize * 3)
					ImGui::SameLine();
			}
		}

		for (auto& directory : std::filesystem::directory_iterator(currentPath))
		{
			std::string fileName = directory.path().filename().u8string();

			if (directory.is_directory()) continue;
			
			ImGui::PushID(id++);
			
			DXShaderResourceView* icon = fileIcon;
			
			std::string extension = directory.path().extension().u8string();
			
			static std::string meshExtensions[]
			{
				".blend", ".fbx", ".obj"
			};
			
			for (int i = 0; i < 3; ++i) {
				if (meshExtensions[i] == extension) {
					icon = meshIcon;
					extension = "MESH";
					break;
				}
			}
			
			ImGui::BeginGroup();
			if (GUI::ImageButton(icon)) {
				spdlog::info("clicked {0}", (void*)icon);
			}
			
			ImGui::TextWrapped(fileName.substr(0, std::min<int>(10, fileName.length())).c_str());
			
			ImGui::EndGroup();
			
			const int width = ImGui::GetColumnWidth();
			
			GUI::DragUIElementString(directory.path().u8string().c_str(), extension.c_str(), icon);
			
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::Text(fileName.c_str());
				ImGui::EndTooltip();
			}
			
			ImGui::PopID();
			if (width > filesize * 3)
				ImGui::SameLine();
		}

		ImGui::End();
	}
}
#endif