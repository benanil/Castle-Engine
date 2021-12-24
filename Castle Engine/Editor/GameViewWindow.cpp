#include "Editor.hpp"
#include "../Main/Event.hpp"
#include "../Engine.hpp"

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
			ImGui::Begin("Game Window", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			
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
			first = false;
	
			ImGui::End();
			ImGui::PopStyleVar();
		}
	}
}