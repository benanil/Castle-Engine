#pragma once
#include <SDL.h>
#include <d3d11.h>
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_dx11.h"
#include "../Main/Event.hpp"
#include <iostream>
#include <functional>
#include <glm/glm.hpp>
#include "../Helper.hpp"
#include "../Main/Event.hpp"

typedef void(*FileCallback)(const char* ptr);

#define HEADER_COLOR { .81f, .6f, 0, 1 }

namespace Editor
{
	void Initialize(SDL_Window* window, ID3D11Device* d3d11Device, ID3D11DeviceContext* d3d11DevCon);
	void AddOnEditor(const Action& action);
	void Render();
	void Clear();
	void DarkTheme();

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
		void TextureField(const char* name, DXShaderResourceView* texture);
		void TextureField(const char* name, const int& texture);
		bool ImageButton(const unsigned int& texture, const float& size = filesize);

		void EnumField(int& value, const char** names, const int& count, const char* label,
		const Action& onSellect = NULL, const ImGuiComboFlags& flags = 0);

		void RightClickPopUp(const char* name, const TitleAndAction* menuItems, const int& count);

		bool DragUIElementString(const char* file, const char* type, const unsigned int& texture);
		void DropUIElementString(const char* type, const FileCallback& callback);

		template<typename T>
		bool DragUIElement(const char* file, const T& type, const unsigned int& texture);
		template<typename T>
		void DropUIElement(const char* type, const std::function<T>& callback);
	}

	struct GameViewWindowData
	{
		ImVec2 PanelPosition;
		ImVec2 WindowScale;

		bool Hovered, Focused;

		Func<Changed2f, float, float> OnScaleChanged;

		DXShaderResourceView* texture;
	};

	namespace GameViewWindow
	{
		void Draw();
		GameViewWindowData& GetData();
	}

}