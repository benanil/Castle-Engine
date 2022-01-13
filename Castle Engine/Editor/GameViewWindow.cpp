#include "Editor.hpp"
#include "../Main/Event.hpp"
#include "../Engine.hpp"
#include "../External/ImGuizmo.h"
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
			}
			
			ImGuizmo::BeginFrame();

			if (ImGui::IsWindowHovered() || ImGui::IsWindowFocused())
			{
				static ImGuizmo::OPERATION operation = ImGuizmo::OPERATION::TRANSLATE;

				if (Engine::GetKeyDown(SDLK_q)) operation = ImGuizmo::OPERATION::ROTATE;
				if (Engine::GetKeyDown(SDLK_w)) operation = ImGuizmo::OPERATION::TRANSLATE;
				if (Engine::GetKeyDown(SDLK_r)) operation = ImGuizmo::OPERATION::SCALE;

				ImVec2 panelSize = ImGui::GetContentRegionAvail();
				ImGuizmo::Enable(true);
				ImGuizmo::SetOrthographic(false);
				ImGuizmo::SetDrawlist();
				ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, panelSize.x, panelSize.y);

				ImGuizmo::Manipulate(data.view, data.projection, operation, ImGuizmo::MODE::LOCAL, data.matrix);

				if (ImGuizmo::IsUsing())
				{
					data.OnManipulated(data.matrix);
				}
			}

			first = false;
	
			ImGui::End();
			ImGui::PopStyleVar();
		}
	}
}
#endif
