#pragma once
#ifndef NEDITOR
#include <SDL.h>
#include <d3d11.h>
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_dx11.h"
#include "../Main/Event.hpp"
#include <iostream>
#include <functional>
#include "../Rendering.hpp"
#include "../Main/Event.hpp"
#include "FontAwesome4.hpp"
#include <filesystem>
#include "../Rendering/Texture.hpp"

typedef void(*FileCallback)(const char* ptr);

#define HEADER_COLOR { .81f, .6f, 0, 1 }


namespace Editor
{
	void Initialize(SDL_Window* window, ID3D11Device* d3d11Device, ID3D11DeviceContext* d3d11DevCon);
	void AddOnEditor(const Action& action);
	void NewFrame();
	void Render();
	void Clear();
	void DarkTheme();
	void PushIconFont();
	void PushLiberationSansFont();
	
	constexpr float filesize = 35;
	constexpr float miniSize = 12.5f;

	struct TitleAndAction
	{
		const char* title;
		Action action;
		TitleAndAction() : title("empty title"), action(nullptr) {}
		TitleAndAction(const char* _title, Action _action) : title(_title), action(_action) {}
	};

	namespace GUI
	{
		void Header(const char* title);
		void TextureField(const char* name, Texture*& texture);
		
		inline bool ImageButton(DXShaderResourceView* texture, const float& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1))
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.30f, 0.30f, 0.30f, 0.65f));
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);
			bool clicked = ImGui::ImageButton((void*)texture, { size , size }, uv0, uv1);
			ImGui::PopStyleColor(1);
			ImGui::PopStyleVar(1); 
			return clicked;
		}

		bool EnumField(int& value, const char** names, const int& count, const char* label,
		const Action& onSellect = NULL, const ImGuiComboFlags& flags = 0);

		void RightClickPopUp(const char* name, const TitleAndAction* menuItems, const int& count);

		bool DragUIElementString(const char* file, const char* type, DXShaderResourceView* texture);
		void DropUIElementString(const char* type, const FileCallback& callback);

		template<typename T>
		bool DragUIElement(const T* file, const char* type, DXShaderResourceView* texture);
		template<typename T>
		void DropUIElement(const char* type, const std::function<T>& callback);
		
		inline bool IconButton(const char* name, const ImVec2& size = ImVec2(0, 0))
		{
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.15f, 1));
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.16f, 0.16f, 0.16f, 0.18f));

			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.3f);
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);
			bool clicked = ImGui::Button(name, size);
			ImGui::PopStyleVar(2);
			ImGui::PopStyleColor(2);

			return clicked;
		}
	}

	struct GameViewWindowData
	{
		ImVec2 PanelPosition;
		ImVec2 WindowScale;

		bool Hovered, Focused;

		EventEmitter<float, float> OnScaleChanged;

		DXShaderResourceView* texture;
	};

	namespace GameViewWindow
	{
		void Draw();
		GameViewWindowData& GetData();
	}

	namespace ResourcesWindow
	{
		std::filesystem::path GetCurrentPath();
		void Initialize();
		void DrawWindow();
	}
}
#endif
















