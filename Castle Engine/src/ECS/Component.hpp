#pragma once
#include <string>
#include <iostream>
#include <SDL.h>
#include "../Main/Event.hpp"
#include "../Transform.hpp"
#include "Entity.hpp"

namespace ECS
{
	class Entity;

	class Component 
	{
	public:
		std::string name;
		Event OnDestroyed;
		Entity* entity;
		Transform* transform;
	public:
		Component() : name(std::string(typeid(*this).name())), transform(new Transform()) {  }
		Component(const std::string& _name) : name(_name) {  }
		
		virtual void SetEntity(Entity* entity) = 0;
		virtual Entity* GetEntity() = 0;
	
		// virtual ~Component() { OnDestroyed() ; };
		virtual void Update(const float& deltaTime) = 0;
#ifndef NEDITOR
		virtual void OnEditor() = 0;
#endif
	};
}