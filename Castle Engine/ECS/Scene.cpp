#include "Scene.hpp"
#define ENTITY_LOOP for (auto& entity : entities)

namespace ECS
{
	Scene::~Scene()
	{
		Unload();
	}

	void Scene::Update() 
	{
		ENTITY_LOOP entity->Update();
	}

	void Scene::UpdateEditor() 
	{
		ENTITY_LOOP entity->UpdateEditor();
	}
	
	Entity* Scene::FindEntityByName(const std::string& name)
	{
		ENTITY_LOOP
		{
			if (entity->name == name) return entity;
		}
	}

	void Scene::Unload()
	{
		ENTITY_LOOP entity->~Entity();
	}

	void Scene::AddEntity(Entity* entity)    { entities.push_back(entity); }
	void Scene::RemoveEntity(Entity* entity) { entities.remove(entity);    }

	namespace SceneManager
	{
		std::list<Scene*> scenes;
		Scene* CurrentScene;

		Scene* GetCurrentScene() { return CurrentScene; }

		void AddScene(Scene* scene) { scenes.push_back(scene); }
		void LoadNewScene() 
		{ 
			scenes.push_back(new Scene()); 
			CurrentScene = scenes.back();
		}
		void LoadScene(const uint8_t& index)
		{
			auto iter = scenes.begin(); 
			for (uint8_t i = 0; index < i; i++, ++iter);
			CurrentScene = *iter;
			CurrentScene->Load();
		}
		
		void LoadScene(const std::string& name)
		{
			auto iter = scenes.begin();
			for (auto& scene : scenes)
			{
				if (scene->name == name)
				{
					scene->Load();
					break;
				}
			}
		}
	}
}
