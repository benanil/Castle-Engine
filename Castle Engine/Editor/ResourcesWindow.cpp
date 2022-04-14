#ifndef NEDITOR

#include "Editor.hpp"
#include <spdlog/spdlog.h>
#include <unordered_map>
#include <SDL.h>
#include "../Input.hpp"
#include "../Rendering/Texture.hpp"
#include "../Helper.hpp"
#include <atomic>
#include <future>
#include <thread>

namespace Editor::ResourcesWindow
{
	class FileRecord
	{
	public:
		mutable std::string path;
		mutable std::string name;
		mutable const char* extension;
		DXShaderResourceView* texture = nullptr;
		FileRecord(std::string _path, std::string _name, const char* _extension) 
			: path(std::move(_path)), 
			  name(std::move(_name)),
			  extension(_extension) {}
	};
	
	class FolderTree
	{
	public:
		FolderTree(std::filesystem::path _path, std::string _name) : path(_path), name(std::move(_name)) {}
	public:
		std::filesystem::path  path;
		FolderTree* parent;
		mutable std::string name;
		std::vector<FolderTree*> folders; // < -- subfolders
		std::vector<FileRecord*> files;
	};

	constexpr uint CppHash  = StringToHash(".cpp");
	constexpr uint HlslHash = StringToHash(".hlsl");
	constexpr uint HppHash  = StringToHash(".hpp");
	constexpr uint MatHash  = StringToHash(".mat");
	constexpr uint blendHash = StringToHash(".blend");
	constexpr uint fbxHash   = StringToHash(".fbx");
	constexpr uint objHash   = StringToHash(".obj");

	std::filesystem::path currentPath;

	DXShaderResourceView* fileIcon, * folderIcon,
					    * meshIcon, * cppIcon   ,
						* hlslIcon, * hppIcon   , * materialIcon;

	FolderTree* rootTree;
	FolderTree* currentTree;

	std::filesystem::path GetCurrentPath() { return currentPath; }

	inline DXShaderResourceView* ExtensionToIcon(std::string& extension)
	{
		switch (StringToHash(extension.c_str())) // StringToHash Function is located in Helper.hpp
		{
		case CppHash:  extension = "CPP";  return cppIcon;
		case HlslHash: extension = "HLSL";  return hlslIcon;
		case HppHash:  extension = "HPP";  return hppIcon;
		case MatHash:  extension = "MAT";  return materialIcon;
		case fbxHash: case objHash: case blendHash: {
			extension = "MESH";
			return materialIcon;
		}
		default: fileIcon;
		}
		return fileIcon;
	}

	FolderTree* CreateTreeRec(FolderTree* parent, const std::filesystem::path& path)
	{
		std::string folderName = path.filename().u8string();
		FolderTree* tree = new FolderTree(path, folderName);
		for (auto& directory : std::filesystem::directory_iterator(path))
		{
			if (directory.is_directory()) continue;
			std::string fileName = directory.path().filename().u8string();
			std::string filePath = directory.path().u8string();
			std::string extension = directory.path().extension().u8string();
			DXShaderResourceView* icon = ExtensionToIcon(extension);
			
			std::string;

			if (extension == ".png" or extension == ".jpg") {
				icon = (new Texture(filePath.c_str()))->resourceView;
				extension = "TEXTURE";
			}
			
			FileRecord* record = new FileRecord(filePath, fileName, extension.c_str());
			record->texture = icon;
			tree->files.push_back(record);
		}
		for (auto& directory : std::filesystem::directory_iterator(path))
		{
			if (!directory.is_directory()) continue;
			tree->folders.push_back(CreateTreeRec(tree, directory.path()));
		}
		tree->parent = parent;
		return tree;
	}

	void Initialize()
	{
		fileIcon = folderIcon =  meshIcon =  cppIcon =  hlslIcon =  hppIcon = 
		materialIcon = fileIcon;
	
		fileIcon     = Texture("Textures/Icons/file.png").resourceView;
		folderIcon   = Texture("Textures/Icons/folder.png").resourceView;
		meshIcon     = Texture("Textures/Icons/mesh.png").resourceView;

		cppIcon      = Texture("Textures/Icons/cpp_icon.png").resourceView;
		hlslIcon     = Texture("Textures/Icons/hlsl_file_icon.png").resourceView;
		hppIcon      = Texture("Textures/Icons/hpp_icon.png").resourceView;
		materialIcon = Texture("Textures/Icons/Material_Icon.png").resourceView;

		currentPath = std::filesystem::current_path();
		
		rootTree = CreateTreeRec(nullptr, currentPath);
		currentTree = rootTree;
	}

	void TreeDrawRec(FolderTree* tree, int& id)
	{
		static auto flags = ImGuiTreeNodeFlags_OpenOnArrow;//: ImGuiTreeNodeFlags_OpenOnArrow;
		
		ImGui::PushID(id++);
		if (ImGui::TreeNodeEx(tree->name.c_str(), flags))
		{
			for (auto& folder : tree->folders)
			{
				TreeDrawRec(folder, id);
			}
			ImGui::TreePop();
		}
		if (ImGui::IsItemClicked()) { currentTree = tree; }
		ImGui::PopID();
	}

	void TreeWindowDraw(int& id)
	{
		static bool Open = true;

		ImGui::Begin("ResourcesTree", &Open, ImGuiWindowFlags_NoTitleBar);

		TreeDrawRec(rootTree, id);

		ImGui::End();
	}

	std::vector<FolderTree*> searchFolders;
	std::vector<FileRecord*> searchFiles  ;
	
	void RecursiveSearch(const char* key, const int len, FolderTree* tree)
	{
		for (auto& folder : tree->folders)
		{
			for (int i = 0; i + len <= folder->name.size(); ++i)
			{
				for (int j = 0; j < len; ++j)
					if (tolower(key[j]) != tolower(folder->name[i + j]))
						goto next_index_folder;
				searchFolders.push_back(folder);
				break;
			next_index_folder: {}
			}
			RecursiveSearch(key, len, folder);
		}

		for (auto& file : tree->files)
		{
			for (int i = 0; i + len <= file->name.size(); ++i)
			{
				for (int j = 0; j < len; ++j)
					if (tolower(key[j]) != tolower(file->name[i + j]))
						goto next_index_file;
				searchFiles.push_back(file);
				break;
			next_index_file: {}
			}
		}
	}

	void SearchProcess(const char* SearchText)
	{
		const int len = strlen(SearchText);
		searchFolders.clear();
		searchFiles  .clear();
		RecursiveSearch(SearchText, len, rootTree);
	}

	void SearchWindow()
	{
		int id = 0;

		for (auto& folder : searchFolders)
		{
			ImGui::PushID(id++);

			if (GUI::ImageButton(folderIcon, Editor::filesize))
			{
				spdlog::info("clicked {0}", folder->name);
				currentPath = folder->path;
				currentTree = folder;
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::Text(folder->name.c_str());
				ImGui::EndTooltip();
			}

			ImGui::TextWrapped("%s", folder->name.c_str());

			ImGui::PopID();
		}

		for (auto& file : searchFiles)
		{
			ImGui::PushID(id++);

			DXShaderResourceView* icon = file->texture;

			GUI::ImageButton(file->texture, Editor::filesize, { 0, 0 }, { 1, 1 });

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
			{
				ShellExecute(0, 0, std::wstring(file->path.begin(), file->path.end()).c_str(), 0, 0, SW_SHOW);
			}

			GUI::DragUIElementString(file->path.c_str(), file->extension, file->texture);

			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::Text(file->name.c_str());
				ImGui::EndTooltip();
			}

			ImGui::TextWrapped("%s", file->name.c_str());

			ImGui::PopID();
			ImGui::NextColumn();
		}

		ImGui::NextColumn();
		ImGui::Columns(1);
		ImGui::End();
	}

	void DrawWindow()
	{
		int id = 0; // for imgui push id
		static bool searching = false;
		
		TreeWindowDraw(id);

		ImGui::Begin("Resources");

		if (GUI::IconButton(ICON_FA_ARROW_LEFT) && currentTree->parent) {
			currentPath = currentPath.parent_path();
			currentTree = currentTree->parent;
		}
		ImGui::SameLine();
		ImGui::Text(ICON_FA_SEARCH);
		ImGui::SameLine();
		static char SearchText[128];
		
		if (ImGui::InputText("Search", SearchText, 128))
		{
			SearchProcess(SearchText);
		}
		
		ImGui::Text(currentPath.u8string().c_str());

		searching = strlen(SearchText);

		ImGui::Text(searching ? "searching" : "not searching");

		static float padding = 3.14f;
		static float thumbnailSize = 64.0f;
		float cellSize = 64.0f + padding;
		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = std::max(1, (int)floor(panelWidth / cellSize)) ;
		ImGui::Columns(columnCount, "resources-columns", false);

		if (searching)
		{
			SearchWindow();
			return;
		}

		FolderTree* folderRec = currentTree;
		// todo scroll and zoom in and out
		for (auto& folder : currentTree->folders)
		{
			ImGui::PushID(id++);

			if (GUI::ImageButton(folderIcon, Editor::filesize))
			{
				spdlog::info("clicked {0}", folder->name);
				currentPath = folder->path;
				folderRec = folder;
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::Text(folder->name.c_str());
				ImGui::EndTooltip();
			}

			ImGui::TextWrapped("%s", folder->name.c_str());

			ImGui::PopID();
			ImGui::NextColumn();
		}

		currentTree = folderRec;
		
		for (int i = 0; i < currentTree->files.size(); ++i)
		{
			auto& file = currentTree->files[i];
			ImGui::PushID(id++);

			DXShaderResourceView* icon = file->texture;

			GUI::ImageButton(file->texture, Editor::filesize, { 0, 0 }, { 1, 1 });

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
			{
				ShellExecute(0, 0, std::wstring(file->path.begin(), file->path.end()).c_str(), 0, 0, SW_SHOW);
			}

			GUI::DragUIElementString(file->path.c_str(), file->extension, file->texture);

			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::Text(file->name.c_str());
				ImGui::EndTooltip();
			}

			ImGui::TextWrapped("%s", file->name.c_str());

			ImGui::PopID();
			ImGui::NextColumn();
		}

		ImGui::Columns(1);

		ImGui::End();
	}
}
#endif