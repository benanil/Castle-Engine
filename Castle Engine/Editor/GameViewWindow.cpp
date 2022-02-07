#include "Editor.hpp"
#include "../Main/Event.hpp"
#include "../Engine.hpp"
#include <filesystem>
#include "../Rendering/Mesh.hpp"
#include "../Rendering/Renderer3D.hpp"
#include "../ECS/ECS.hpp"

#ifndef NEDITOR

namespace Editor
{
	namespace GameViewWindow
	{
		GameViewWindowData data;
	
		GameViewWindowData& GetData() LAMBDAR(data)

		ImVec2 newScale, newPosition;

		void ChangeScale()
		{
			data.PanelPosition = newPosition;
			data.WindowScale = newScale;
			data.OnScaleChanged(newScale.x, newScale.y);
		}

		void FileCallback(const char* file)
		{
			ECS::Entity* entity = new ECS::Entity();
			ECS::SceneManager::GetCurrentScene()->AddEntity(entity);
			MeshRenderer* renderer = MeshLoader::LoadMesh(file);
			Renderer3D::AddMeshRenderer(renderer);
			renderer->SetEntity(entity);
			entity->AddComponent((ECS::Component*)renderer);
			std::cout << "dropped" << std::endl;
			std::cout << file << std::endl;
		}

		void Draw()
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
			static const int flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground;
			ImGui::Begin("Game Window", nullptr, flags);
			
			data.Hovered = ImGui::IsWindowHovered();
			data.Focused = ImGui::IsWindowFocused();
		
			newScale    = ImGui::GetWindowSize();
			newPosition = ImGui::GetWindowPos(); 
			
			if (data.WindowScale.x != newScale.x || data.WindowScale.y != newScale.y || 
				data.PanelPosition.x != newPosition.x || data.PanelPosition.y != newPosition.y)
			{
				Engine::AddEndOfFrameEvent(ChangeScale);
			}

			static bool first = true;
			
			if (!first)
			{
				ImGui::Image(data.texture, data.WindowScale, { 1, 1 }, {0, 0});
				GUI::DropUIElementString("MESH", FileCallback);
			}

			first = false;
	
			ImGui::End();
			ImGui::PopStyleVar();
		}
	}
}
#endif
