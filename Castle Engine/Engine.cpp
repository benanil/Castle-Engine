//Include and link appropriate libraries and headers//
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")

#include <windows.h>
#include "DirectxBackend.hpp"
#include <cassert>

#include "SDL.h"
#undef SDL_HAS_VULKAN 
#include <SDL_syswm.h>
#ifndef NEDITOR
#   include "Editor/Editor.hpp"
#endif

#include "Rendering.hpp"
#include "Rendering/All.hpp"

#include "Timer.hpp"
#include "Main/Time.hpp"
#include "Input.hpp"
#include "Engine.hpp"
#include "ECS/ECS.hpp"
#include "FreeCamera.hpp"
#include <iostream>

using namespace ECS;

namespace Engine
{
    //Global Declarations - Others//
    SDL_Window* window;
    HWND hwnd = NULL;
    FreeCamera* freeCamera;

    Event EndOfFrameEvents;
    EventEmitter<int, int> ScreenScaleChanged;

    void MainWindow();
    void CleanUp();
    void Start();
    void InitScene();
    void WindowSizeChanged();
}

void Engine::AddEndOfFrameEvent(Action action) { EndOfFrameEvents.Add(action);}
void Engine::AddWindowScaleEvent(FunctionAction<void, int, int>::Type act) { ScreenScaleChanged.Bind(act); }

HWND Engine::GetHWND() { return hwnd; }

SDL_Window* Engine::GetWindow() { return window; };

void Engine::DirectXCheck(const HRESULT& hr, const int line, const char* file)
{																					
	if (FAILED(hr)) {																
		std::string description = std::string("UNKNOWN ERROR!\n") + std::string(file) + " at line: " + std::to_string(line);
		std::cout << description << "hresult is: " << hr << std::endl;
		SDL_ShowSimpleMessageBox(SDL_MessageBoxFlags::SDL_MESSAGEBOX_ERROR, "DX ERROR!", description.c_str(), window);
		assert(1);
	}	
}		
void Engine::DirectXCheck(const HRESULT& hr, const char* message, const int line, const char* file)               
{																									              
	if (FAILED(hr)){																					              
		std::string description = std::string(message) + std::string(file) + " at line: " + std::to_string(line);	  
		std::cout << description << "hresult is: " << hr << std::endl;												  
		SDL_ShowSimpleMessageBox(SDL_MessageBoxFlags::SDL_MESSAGEBOX_ERROR, "DX ERROR!", description.c_str(), window);
		assert(1);																									
	}																												
}

#ifndef NEDITOR
void Engine::MainWindow()
{
    ImGui::Begin("Settings");

    if (ImGui::CollapsingHeader("Camera")) freeCamera->EditorUpdate();

	Terrain::OnEditor();
	Renderer3D::OnEditor();
	
    ImGui::End();

    Editor::GameViewWindow::Draw();

    if (ECS::SceneManager::GetCurrentScene())
    {
        ECS::SceneManager::GetCurrentScene()->UpdateEditor();
    }
}
#endif

void Engine::InitScene()
{
	freeCamera = new FreeCamera(90, Width / Height, 0.1f, 90'000.0f);

	Renderer3D::Initialize(DirectxBackend::GetDevice(), DirectxBackend::GetDeviceContext(), DirectxBackend::GetMSAASamples(), freeCamera);
#ifndef NEDITOR
    Editor::Initialize(window, DirectxBackend::GetDevice(), DirectxBackend::GetDeviceContext());
    Editor::DarkTheme();
    Editor::AddOnEditor(MainWindow);
#endif
	SceneManager::LoadNewScene();

	Entity* firstEntity = new Entity();
	MeshRenderer* mesh = MeshLoader::LoadMesh("Models/sponza.obj");
	mesh->SetEntity(firstEntity);

	Renderer3D::AddMeshRenderer(mesh);
	SceneManager::GetCurrentScene()->AddEntity(firstEntity);
	mesh->SetEntity(firstEntity);
	firstEntity->AddComponent((ECS::Component*)mesh);
}

void Engine::WindowSizeChanged()
{
    WindowSize windowSize = DirectxBackend::GetWindowSize();
#ifdef NEDITOR
    if (freeCamera) {
        freeCamera->UpdateProjection((float)depthDesc.Width / depthDesc.Height);
    }
    if (renderTexture) {
        renderTexture->Invalidate(depthDesc.Width, depthDesc.Height);
    }
#else
    auto& viewWindowdata = Editor::GameViewWindow::GetData();
    viewWindowdata.OnScaleChanged.Invoke(windowSize.Width, windowSize.Height);

    static bool firstTime = true;
    if (!firstTime)
    {
        ImGuiViewport* imViewport = ImGui::GetMainViewport();
        imViewport->Size.x = windowSize.Width;
        imViewport->Size.x = windowSize.Height;
    }
    firstTime = false;
#endif
    ScreenScaleChanged.Invoke((int)windowSize.Width, (int)windowSize.Height);
}

void Engine::Start()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        printf("Error: %s\n", SDL_GetError());
        assert(1, "stl initialization failed");
    }

    const SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    window = SDL_CreateWindow("Castle Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Width, Height, window_flags);
    SDL_SysWMinfo wmInfo{};
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(window, &wmInfo);

    hwnd = (HWND)wmInfo.info.win.window;

    DirectxBackend::InitializeDirect3d11App(window);    //Initialize Direct3D

    InitScene();    //Initialize our scene
    // Main loop
    bool done = false;

    while (!done)
    {
        SDL_Event event;

        Time::Tick(SDL_GetTicks());

        while (SDL_PollEvent(&event))
        {
#ifndef NEDITOR
            ImGui_ImplSDL2_ProcessEvent(&event);
#endif
            Input::Proceed(&event, done);
            SceneManager::GetCurrentScene()->ProceedEvent(&event);
            freeCamera->Update();
            done |= event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window);
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED && event.window.windowID == SDL_GetWindowID(window))
            {
                // Release all outstanding references to the swap chain's buffers before resizing.
                DirectxBackend::CreateRenderTarget(window);
                WindowSizeChanged();
            }
        }

		freeCamera->Update();
        SceneManager::GetCurrentScene()->Update(Time::GetDeltaTime());
        Renderer3D::DrawScene();
        EndOfFrameEvents.Invoke();
        EndOfFrameEvents.Clear();
    }
    CleanUp();
}

void Engine::CleanUp()
{
    //Release the COM Objects we created
#ifndef NEDITOR
    Editor::Clear();
#endif
    Terrain::Dispose();
    Renderer3D::Dispose();
    DirectxBackend::Destroy();
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main(int, char**)
{
    Engine::Start();
    return 0;
}