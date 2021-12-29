#pragma once
#include "Component.hpp"
#include <list>
#include <cstdint>
#include "../Main/Event.hpp"
#include <memory>
#include "../Transform.hpp"
#include <SDL.h>

#ifndef NEDITOR
	#include "../Editor/Editor.hpp"
#endif

namespace ECS
{
	class Entity
	{
	public:
		std::string name;
		Event OnDestroy;
		std::unique_ptr<Transform> transform;
		std::list<Component*> components;
	public:
		Entity() : name(std::string("Entity")), transform(std::make_unique<Transform>()) { };
		Entity(const std::string& _name) { name = _name; };
		~Entity() ;

		void Update(const float& deltaTime);
		// void ProceedEvent(const SDL_Event* _event);
#ifndef NEDITOR
		void UpdateEditor();
#endif
		Component* GetComponent(const uint16_t& index);
		template<typename TComp> TComp* GetComponent();

		void AddComponent(Component* component);
		void RemoveComponent(Component* component);
		void RemoveComponent(const uint16_t& index);

		template<typename TComp> void RemoveComponent();
		template<typename TComp> bool TryGetComponent(TComp** component);
		template<typename TComp> bool HasComponent();
		bool HasComponent(Component* component);
	};

	class TestComponent : Component
	{
	public:
		void SetEntity(Entity* _entity) { entity = _entity; }
		Entity* GetEntity() { return entity; }

		void Update(const float&) {}
#ifndef NEDITOR
		void OnEditor()
		{
			ImGui::Text("Test Component");
		}
#endif
	};
}
