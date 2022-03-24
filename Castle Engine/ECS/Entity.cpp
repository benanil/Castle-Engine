#include "Entity.hpp" 

namespace ECS
{
	Entity::~Entity()
	{
		OnDestroy();
		components.Iterate([](Component* comp) { comp->~Component(); });
	};

	void Entity::Update(const float& deltaTime) {
		static float DeltaTime = deltaTime;
		components.Iterate([](Component* comp) { comp->Update(DeltaTime); });
	}

#ifndef NEDITOR
	void Entity::UpdateEditor()
	{
		ImGui::TextColored(HEADER_COLOR, name.c_str());
		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_Bullet))
		{
			transform->OnEditor();
		}
		static int PushID = 0;
		components.Iterate([](Component* comp) 
		{
			ImGui::PushID(PushID++);
			if (ImGui::CollapsingHeader(comp->name.c_str(), ImGuiTreeNodeFlags_Bullet))
			{
				comp->OnEditor();
			}

			ImGui::PopID();
		});

		PushID = 0;
	}
#endif

	Component* Entity::GetComponent(uint16_t index)
	{
		return components[index];
	}

	template<typename TComp> TComp* Entity::GetComponent()
	{
		return components.FindNodeByType<TComp>();
	}

	void Entity::AddComponent(Component* component)
	{
		components.AddFront(component);
	}

	void Entity::RemoveComponent(Component* component)
	{
		if (component == nullptr) return;
		if (!HasComponent(component)) return;
		components.Remove(component);
		delete component;
	}

	template<typename TComp> void Entity::RemoveComponent()
	{
		TComp* component = components.Remove<TComp>(components.FindNodeByType<TComp>());
		component->~Component();
		delete component;
	}

	bool Entity::HasComponent(Component* component)
	{
		return components.FindNodeFromPtr(component) != nullptr;
	}

	template<typename TComp> bool Entity::HasComponent()
	{
		return components.FindNodeByType<TComp>() != nullptr;
	}

	template<typename TComp>
	bool Entity::TryGetComponent(TComp** component)
	{
		return components.TryGetData<TComp>(component);
	}
}