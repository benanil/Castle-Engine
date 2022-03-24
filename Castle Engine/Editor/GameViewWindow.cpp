#include "Editor.hpp"
#include "../Main/Event.hpp"
#include "../Engine.hpp"
#include <filesystem>
#include "../Rendering/Mesh.hpp"
#include "../Rendering/Renderer3D.hpp"
#include "../ECS/ECS.hpp"
#include "ImGuizmo.h"
#include "../Rendering/Renderer3D.hpp"
#include "../Input.hpp"
#include "../Main/Time.hpp"
#include "spdlog/spdlog.h"

#ifndef NEDITOR

using namespace ECS;

namespace Editor
{
	namespace GameViewWindow
	{
		GameViewWindowData data;
	
		GameViewWindowData& GetData() LAMBDAR(data)

		ImVec2 newScale, newPosition;
		ImGuizmo::OPERATION operation;

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
				ImGui::Image(data.texture, data.WindowScale);
				GUI::DropUIElementString("MESH", FileCallback);
			}

			const FreeCamera* freeCamera = Renderer3D::GetCamera();
			Entity* currEntity = SceneManager::GetCurrentScene()->GetCurrentEntity();
			if (currEntity)
			{
				if (ImGui::IsWindowHovered())
				{
					if (Input::GetKeyDown(KeyCode::W)) operation = ImGuizmo::OPERATION::TRANSLATE;
					if (Input::GetKeyDown(KeyCode::Q)) operation = ImGuizmo::OPERATION::ROTATE;
					if (Input::GetKeyDown(KeyCode::R)) operation = ImGuizmo::OPERATION::SCALE;
				}
				float recomposed[16];
				glm::vec3 position = currEntity->transform->GetPosition();
				glm::vec3 scale = currEntity->transform->GetScale();
				glm::vec3 rotation = currEntity->transform->GetEulerDegree();
			
				ImGuizmo::RecomposeMatrixFromComponents(&position.x, &rotation.x, &scale.x, recomposed);
			
				ImVec2 panelSize = ImGui::GetWindowSize();
			
				ImGuiIO& io = ImGui::GetIO();
				ImGuizmo::Enable(true);
				ImGuizmo::SetOrthographic(false);
				ImGuizmo::SetDrawlist();
				ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, panelSize.x, panelSize.y);
			
				if (ImGuizmo::Manipulate(&freeCamera->GetView()._11, &freeCamera->GetProjection()._11,
					operation, ImGuizmo::MODE::LOCAL, recomposed))
				{
					ImGuizmo::DecomposeMatrixToComponents(recomposed, &position.x, &rotation.x, &scale.x);
					
					currEntity->transform->SetScale(scale, false);
					currEntity->transform->SetPosition(position, false);
					currEntity->transform->SetEulerDegree(rotation, true);
				}
			}

			first = false;
	
			ImGui::End();
			ImGui::PopStyleVar();
		}
	}
}
#endif
