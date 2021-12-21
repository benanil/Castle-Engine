#pragma once
#include <string>
#include "../Main/Event.hpp"

namespace ECS
{
	class Component 
	{
	public:
		std::string name;
		Event OnDestroyed;
	public:
		Component() { name = std::string(typeid(this).name()); };
		virtual ~Component() { OnDestroyed() ; };
		virtual void Update() = 0;
		virtual void OnEditor() = 0;
	};
}