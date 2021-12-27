#pragma once
#include "ECS.hpp"
#include <list>
#include "../Main/Event.hpp"
#include <cstdint>
#include <SDL.h>

namespace ECS
{
	class Scene
	{
		std::list<Entity*> entities;
	public:
		std::string name;
	public:
		Scene() : name(std::string("New Scene")) {};
		Scene(const std::string& _name) : name(_name) {};
		~Scene();
		void Update(const float& deltaTime);
		void ProceedEvent(const SDL_Event* _event);
		void Unload();
		void Load()   {};
		void Start()  {};
		void UpdateEditor();
		void AddEntity(Entity* entitiy);
		void RemoveEntity(Entity* entitiy);
		Entity* FindEntityByName(const std::string& name);
	};

	namespace SceneManager
	{
		Scene* GetCurrentScene();

		void AddScene(Scene* scene);
		void LoadNewScene();
		void LoadScene(const uint8_t& index);
		void LoadScene(const std::string& name);
	}
}