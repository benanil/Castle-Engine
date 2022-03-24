#ifndef NEDITOR

#include "Editor.hpp"
#include <spdlog/spdlog.h>
#include <unordered_map>
#include <SDL.h>
#include "../Input.hpp"
#include "../Rendering/Texture.hpp"
#include "../Helper.hpp"

// custom hashmap example
// namespace Editor::ResourcesWindow
// {
// 	struct IconAndFile
// 	{
// 		 icon;
// 		std::string file;
// 	};
// }
// namespace std
// {
// 	template <>
// 	struct hash<Editor::ResourcesWindow::IconAndFile>
// 	{
// 		std::size_t operator()(const Editor::ResourcesWindow::IconAndFile& c) const
// 		{
// 			std::size_t result = 0;
// 			hash_combine(result, c.icon);
// 			hash_combine(result, c.file);
// 			return result;
// 		}
// 	};
// }

namespace Editor::ResourcesWindow
{
	constexpr uint CppHash  = StringToHash(".cpp");
	constexpr uint HlslHash = StringToHash(".hlsl");
	constexpr uint HppHash  = StringToHash(".hpp");
	constexpr uint MatHash  = StringToHash(".mat");
	
	std::filesystem::path currentPath;

	DXShaderResourceView* fileIcon, * folderIcon,
					    * meshIcon, * cppIcon   ,
						* hlslIcon, * hppIcon   , * materialIcon;

	std::unordered_map<uint, DXShaderResourceView*> image_map;

	std::filesystem::path GetCurrentPath()
	{
		return currentPath;
	}

	void Initialize()
	{
		fileIcon   = Texture("Textures/Icons/file.png").resourceView;
		folderIcon = Texture("Textures/Icons/folder.png").resourceView;
		meshIcon   = Texture("Textures/Icons/mesh.png").resourceView;

		cppIcon      = Texture("Textures/Icons/cpp_icon.png").resourceView;
		hlslIcon     = Texture("Textures/Icons/hlsl_file_icon.png").resourceView;
		hppIcon      = Texture("Textures/Icons/hpp_icon.png").resourceView;
		materialIcon = Texture("Textures/Icons/Material_Icon.png").resourceView;

		currentPath = std::filesystem::current_path();
	}

	void DrawWindow()
	{
		ImGui::Begin("Resources");

		if (GUI::IconButton(ICON_FA_ARROW_LEFT)) {
			currentPath = currentPath.parent_path();
		}
		ImGui::SameLine();
		ImGui::Text(ICON_FA_SEARCH);
		ImGui::SameLine();
		static char SearchText[32];
		ImGui::InputText("Search", SearchText, 32);

		// todo: make path static & add oppen close folders & back button & dragg and drop
		int id = 0; // for imgui push id

		ImGui::Separator();

		static float padding = 3.14f;
		static float thumbnailSize = 64.0f;
		float cellSize = 64.0f + padding;
		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = std::max(1, (int)floor(panelWidth / cellSize)) ;
		ImGui::Columns(columnCount, "resources-columns", false);

		// todo scroll and zoom in and out

		for (auto& directory : std::filesystem::directory_iterator(currentPath))
		{
			std::string fileName = directory.path().filename().u8string();

			if (directory.is_directory()) {
				ImGui::PushID(id++);

				if (GUI::ImageButton(folderIcon, Editor::filesize))
				{
					spdlog::info("clicked {0}", fileName);
					currentPath = directory;
				}
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::Text(fileName.c_str());
					ImGui::EndTooltip();
				}
			
				ImGui::TextWrapped("%s", fileName.substr(0, std::min<int>(10, fileName.length())).c_str());

				ImGui::PopID();
				ImGui::NextColumn();
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

			switch (StringToHash(extension.c_str())) // StringToHash Function is located in Helper.hpp
			{
				case CppHash  : icon = cppIcon;      extension = "CPP";  break;
				case HlslHash : icon = hlslIcon;     extension = "HLSL"; break;
				case HppHash  : icon = hppIcon;      extension = "HPP";  break;
				case MatHash  : icon = materialIcon; extension = "MAT" ; break;
				default: break;
			}

			if (extension == ".png" || extension == ".jpg")
			{
				std::string file = currentPath.u8string() + "\\" + fileName;
				const uint FileHash = StringToHash((file).c_str());
				
				if (image_map.count(FileHash) != 0) { // most of the time this will work so we optimize little bit here
					icon = image_map[FileHash];
				}
				else {// if key is not exist
					std::string file = currentPath.u8string() + "\\" + fileName;
					Texture* newIcon = new Texture(file.c_str());
					image_map.insert(std::make_pair(FileHash, newIcon->resourceView));
					icon = newIcon->resourceView;
				}
			}

			GUI::ImageButton(icon, Editor::filesize, { 0, 0 }, { 1, 1 });
			
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
			{
				std::string file = currentPath.u8string() + "\\" + fileName;
				ShellExecute(0, 0, std::wstring(file.begin(), file.end()).c_str(), 0, 0, SW_SHOW);
			}

			GUI::DragUIElementString(directory.path().u8string().c_str(), extension.c_str(), icon);

			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::Text(fileName.c_str());
				ImGui::EndTooltip();
			}

			ImGui::TextWrapped("%s", fileName.substr(0, std::min<int>(15, fileName.length())).c_str());
			
			ImGui::PopID();
			ImGui::NextColumn();
		}

		ImGui::Columns(1);

		ImGui::End();
	}
}
#endif