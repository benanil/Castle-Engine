#include "Scene.hpp"
#include "Entity.hpp"
#include "../Rendering/Mesh.hpp"
#include "../Editor/FontAwesome4.hpp"
#ifndef NEDITOR
	#include "../Editor/Editor.hpp"
#endif

namespace ECS
{
	Scene::~Scene()
	{
		Unload();
	}

	void Scene::Update(const float& deltaTime) 
	{
		for (auto& entity : entities) entity->Update(deltaTime);
	}

	// todo fix these
	static void NewEntity()
	{
		SceneManager::GetCurrentScene()->AddEntity(new Entity(std::string("NewEntity")));
	}

	static void NewCube()
	{
		Entity* entity = new Entity();
		Mesh* cubeMesh = MeshLoader::LoadMesh(std::string("Models/cube.obj"));
		MeshRenderer* renderer = new MeshRenderer(cubeMesh);
		Renderer3D::AddMeshRenderer(renderer);
		SceneManager::GetCurrentScene()->AddEntity(entity);
		renderer->SetEntity(entity);
		entity->AddComponent((ECS::Component*)renderer);
	}

	static void NewMeshRenderer()
	{
		SceneManager::GetCurrentScene()->AddEntity(new Entity(std::string("Mesh Renderer")));
	}

	static Entity* currEntity = nullptr;
	
	Entity* Scene::GetCurrentEntity() { return currEntity; };
	

	static void AddComponent()
	{
		if (!currEntity) return;

		TestComponent* testComponent = new TestComponent();
		currEntity->AddComponent((Component*)testComponent);
		testComponent->SetEntity(currEntity);
	}

#ifndef NEDITOR
	void Scene::UpdateEditor() 
	{
		static int PushID = 0;

		ImGui::Begin("Hierarchy");
		ImGui::Indent();

		for (auto& entity : entities)
		{
			static auto flags = currEntity == entity ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_OpenOnArrow;
			ImGui::PushID(PushID++);
			
			if (ImGui::TreeNodeEx((ICON_FA_CUBE + entity->name).c_str(), flags))
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
			{ "New Cube", NewCube }
		};
		
		Editor::GUI::RightClickPopUp("Add Object", hierarchyActions, 2);
		ImGui::Unindent();

		ImGui::End();

		ImGui::Begin("Inspector");
		
		if (currEntity)
		{
			currEntity->UpdateEditor();
		}

		static const Editor::TitleAndAction actions[] =
		{
			{ "MeshRenderer", NewMeshRenderer },
			{ "Component", AddComponent}
		};

		Editor::GUI::RightClickPopUp("AddComponent", actions, 2);

		ImGui::End();
	}
#endif
	
	Entity* Scene::FindEntityByName(const std::string& name)
	{
		for (auto& entity : entities)
		{
			if (entity->name == name) return entity;
		}
	}

	void Scene::Unload()
	{
		for (auto& entity : entities) entity->~Entity();
		entities.clear();
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
		void LoadScene(uint8_t index)
		{
			if (CurrentScene) { CurrentScene->Unload(); }
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
					CurrentScene = scene;
					scene->Load();
					break;
				}
			}
		}
	}
}
