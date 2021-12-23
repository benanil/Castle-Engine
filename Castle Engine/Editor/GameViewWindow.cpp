#include "Editor.hpp"

namespace Editor
{
	namespace GameViewWindow
	{
		ImVec2 PanelPosition;
		ImVec2 WindowScale;
	
		bool Hovered, Focused;
		
		DXShaderResourceView* texture;
	
		bool GetHovered() LAMBDAR(Hovered)
		bool GetFocused() LAMBDAR(Focused)
		
		ImVec2 GetPanelScale()    LAMBDAR(WindowScale)
		ImVec2 GetPanelPosition() LAMBDAR(PanelPosition)
	
		void SetTexture(DXShaderResourceView* _texture) { texture = _texture; }
	
		void Draw()
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
			ImGui::Begin("Game Window", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			
			Hovered = ImGui::IsWindowHovered();
			Focused = ImGui::IsWindowFocused();
	
			PanelPosition = ImGui::GetWindowPos();
			WindowScale = ImGui::GetWindowSize();
	
			ImGui::Image(texture, WindowScale);
	
			ImGui::End();
			ImGui::PopStyleVar();
		}
	}
}