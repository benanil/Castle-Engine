#pragma once
#include <string>
#include "../Main/Event.hpp"
#include <iostream>
#include <SDL.h>
#include "Entity.hpp"

namespace ECS
{
	class Entity;
	class Transform;

	class Component 
	{
	public:
		std::string name;
		Event OnDestroyed;
		Entity* entity;
		Transform* transform;
	public:
		Component() : name(std::string(typeid(this).name())) {  }
		Component(const std::string& _name) : name(_name) {  }
		
		virtual void SetEntity(Entity* entity) = 0;
		virtual Entity* GetEntity() = 0;
	
		virtual ~Component() { OnDestroyed() ; };
		virtual void Update(const float& deltaTime) = 0;
		virtual void OnEditor() = 0;
	};
}