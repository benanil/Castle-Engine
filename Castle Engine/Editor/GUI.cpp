#include "Editor.hpp"
// this code mostly copied from ZargoEngine's GUI class

namespace Editor
{
	void GUI::Header(const char* title)
	{
		static const ImVec4 Orange{1.0f, 0.68f ,0.0f ,1.0f };
		ImGui::GetFont()->FontSize += 3;
		ImGui::TextColored(Orange, title);
		ImGui::GetFont()->FontSize -= 3;
	}

	void GUI::TextureField(const char* name, const int& texture)
	{
		ImGui::Text(name);
		ImGui::SameLine();
		ImGui::Image((void*)texture, { filesize, filesize }, { 0, 1 }, { 1, 0 });
		FileCallback callback = [](const char* ptr) {};
		DropUIElementString("Texture", callback);
	}

	void GUI::TextureField(const char* name, DXShaderResourceView* texture)
	{
		ImGui::Text(name);
		ImGui::SameLine();
		ImGui::Image(texture, { filesize, filesize }, { 0, 1 }, { 1, 0 });
		
		FileCallback callback = [](const char* ptr){};
		DropUIElementString("Texture", callback);
	}
	
	bool GUI::ImageButton(const unsigned int& texture, const float& size)
	{
		return ImGui::ImageButton((void*)texture, { size , size }, { 1, 0 }, { 0, 1 });
	}

	// Usage:
	// static int value = 0;
	// const char* names[3]{ "enum0", "enum1", "enum2" };
	// GUI::EnumField(value, names, 3, "TestEnum", NULL, 0);
	/// <summary> note this is not usefull for bitfields | flags </summary> 
	/// <param name="count"> number of names </param>
	void GUI::EnumField(int& value, const char** names, const int& count, const char* label,
						const Action& onSellect, const ImGuiComboFlags& flags)
	{
		if (ImGui::BeginCombo(label, names[value], flags))
		{
			for (char i = 0; i < count; i++)
			{
				if (ImGui::Selectable(names[i], i == value))
				{
					value = i;
					if (onSellect) onSellect();
				}
			}
			ImGui::EndCombo();
		}
	}


	void GUI::RightClickPopUp(const char* name, const TitleAndAction* menuItems, const int& count)
	{
		if (ImGui::BeginPopupContextWindow(name))
		{
			for (char i = 0; i < count; i++)
			{
				if (ImGui::MenuItem(menuItems[i].title))
				{
					menuItems[i].action();
				}
			}
			ImGui::EndPopup();
		}
	}

	bool GUI::DragUIElementString(const char* file, const char* type, const unsigned int& texture)
	{
		if (ImGui::IsItemFocused() && ImGui::IsAnyMouseDown() && ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload(type, file, strlen(file));
			if (texture != -1)
			{
				ImGui::Image((void*)texture, { filesize, filesize }, { 0, 1 }, { 1, 0 });
			}
			ImGui::EndDragDropSource();
			return true;
		}
		return false;
	}

	void GUI::DropUIElementString(const char* type, const FileCallback& callback)
	{
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payloadPtr = ImGui::AcceptDragDropPayload(type))
			{
				if (payloadPtr->Data)
				{
					callback((char*)payloadPtr->Data);
				}
			}
		}
	}

	template<typename T>
	inline bool GUI::DragUIElement(const char* file, const T& type, const unsigned int& texture)
	{
		if (ImGui::IsItemFocused() && ImGui::IsAnyMouseDown() && ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload(type, file, sizeof(T));
			if (texture != -1)
			{
				ImGui::Image((void*)texture, { filesize, filesize }, { 0, 1 }, { 1, 0 });
			}
			ImGui::EndDragDropSource();
			return true;
		}
		return false;
	}
	
	template<typename T>
	inline void GUI::DropUIElement(const char* type, const std::function<T>& callback)
	{
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payloadPtr = ImGui::AcceptDragDropPayload(type))
			{
				if (payloadPtr->Data)
				{
					callback((char*)payloadPtr->Data);
					std::cout << "dropped data type of: " << typeid(T).name << std::endl;
				}
			}
		}
	}
}