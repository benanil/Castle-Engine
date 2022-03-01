#include "Entity.hpp" 

namespace ECS
{
	Entity::~Entity() 
	{
		OnDestroy();
		for (auto& comp : components) comp->~Component();
	};

	void Entity::Update(const float& deltaTime) {
		for (auto& comp : components) comp->Update(deltaTime);
	}

#ifndef NEDITOR
	void Entity::UpdateEditor() 
	{
		ImGui::TextColored(HEADER_COLOR, name.c_str());
		if (ImGui::CollapsingHeader("Transform"))
		{
			transform->OnEditor();
		}
		static int PushID = 0;

		for (auto& comp : components)
		{
			ImGui::PushID(PushID++);
			if (ImGui::CollapsingHeader(comp->name.c_str()))
			{
				comp->OnEditor();
			}

			ImGui::PopID();
		}
		PushID = 0;
	}
#endif

	Component* Entity::GetComponent(uint16_t index)
	{
		auto begin = components.begin();
		for (uint16_t i = 0; i < index; i++, ++begin);
		return *begin;
	}
	
	template<typename TComp> TComp* Entity::GetComponent()
	{
		for (auto& component : components)
			if (dynamic_cast<TComp>(component))
				return component;
		return nullptr;
	}
	
	void Entity::AddComponent(Component* component)
	{
		components.push_back(component);
	}

	void Entity::RemoveComponent(Component* component)
	{
		if (component == nullptr) return;
		if (!HasComponent(component)) return;
		components.remove(component);
		delete component;
	}

	void Entity::RemoveComponent(uint16_t index)
	{
		if (index > components.size()) return;

		Component* component;
		auto iter = components.begin();

		for (uint16_t i = 0; i < index; i++, ++iter)
		{
			if (i == index)
			{
				component = *iter;
				break;
 			}
		}
		components.remove(component);
		component->~Component();
		delete component;
	}
	
	template<typename TComp> void Entity::RemoveComponent()
	{
		Component* component;
		
		for (auto& iter : components)
		{
			if (dynamic_cast<TComp>(*iter))
			{
				*component = *iter;
				break;
			}
		}
		if (component != nullptr)
		components.remove(component);
		delete component;
	}

	bool Entity::HasComponent(Component* component)
	{
		for (auto& comp : components)
			if (comp == component)
				return true;
		return false;
	}

	template<typename TComp> bool Entity::HasComponent()
	{
		for (auto& component : components)
			if (dynamic_cast<TComp>(component))
				return true;
		return false;
	}

	template<typename TComp>
	bool Entity::TryGetComponent(TComp** component)
	{
		for (auto& comp : components)
		{
			if (comp == component)
			{
				*component = comp;
				return true;
			}
		}
		return false;
	}
}

