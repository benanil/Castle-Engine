#include "Entity.hpp"
#define LOOP_COMPONENTS for (auto& comp : components)

namespace ECS
{
	Entity::~Entity() 
	{
		OnDestroy();
		LOOP_COMPONENTS comp->~Component();
	};

	void Entity::Update() {
		LOOP_COMPONENTS comp->Update();
	}

	void Entity::UpdateEditor() 
	{
#ifndef NEDITOR
		ImGui::TextColored(HEADER_COLOR, name.c_str());
		if (ImGui::CollapsingHeader("Transform"))
		{
			transform->OnEditor();
		}
		LOOP_COMPONENTS comp->OnEditor();
#endif
	}

	Component* Entity::GetComponent(const uint16_t& index)
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
	}

	void Entity::RemoveComponent(const uint16_t& index)
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

