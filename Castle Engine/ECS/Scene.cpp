#include "Scene.hpp"
#include "Entity.hpp"

#ifndef NEDITOR
	#include "../Editor/Editor.hpp"
#endif

#define ENTITY_LOOP for (auto& entity : entities)

namespace ECS
{
	Scene::~Scene()
	{
		Unload();
	}

	void Scene::Update(const float& deltaTime) 
	{
		ENTITY_LOOP entity->Update(deltaTime);
	}

	void Scene::ProceedEvent(const SDL_Event* _event)
	{
		// ENTITY_LOOP entity->ProceedEvent(_event);
	}

	static void Test()
	{
		std::cout << "Test" << std::endl;
	}

	static void NewEntity()
	{
		SceneManager::GetCurrentScene()->AddEntity(new Entity("NewEntity"));
	}

	static void NewCube()
	{
		SceneManager::GetCurrentScene()->AddEntity(new Entity("Cube"));
	}

	static Entity* currEntity = nullptr;
	
	static void AddComponent()
	{
		if (!currEntity) return;

		TestComponent* testComponent = new TestComponent();
		currEntity->AddComponent((Component*)testComponent);
		testComponent->SetEntity(currEntity);
	}

	void Scene::UpdateEditor() 
	{
#ifndef NEDITOR
		static int PushID = 0;

		ImGui::Begin("Hierarchy");
		
		for (auto& entity : entities)
		{
			static auto flags = currEntity == entity ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_OpenOnArrow;
			ImGui::PushID(PushID++);
			
			if (ImGui::TreeNodeEx(entity->name.c_str(), flags))
			{
				ImGui::TreePop();
			}
			
			if (ImGui::IsItemClicked())
			{
				currEntity = entity;
			}
			
			ImGui::PopID();
		}
		
		static const Editor::TitleAndAction hierarchyActions[] =
		{
			{ "New Entity", NewEntity},
			{ "New Cube", NewCube },
			{ "New Sphere", Test }
		};
		
		Editor::GUI::RightClickPopUp("Add Object", hierarchyActions, 2);

		ImGui::End();

		ImGui::Begin("Inspector");
		
		if (currEntity)
		{
			currEntity->UpdateEditor();
		}

		static const Editor::TitleAndAction actions[] =
		{
			{ "MeshRenderer", Test },
			{ "Component", AddComponent}
		};

		Editor::GUI::RightClickPopUp("AddComponent", actions, 2);

		ImGui::End();
#endif
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
