//Include and link appropriate libraries and headers//
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")

#define VC_EXTRALEAN    
#define NOMINMAX
#include <windows.h>
#include <cassert>
#include <iostream>

#undef SDL_HAS_VULKAN 
#include <SDL_syswm.h>
#include <SDL_surface.h>
#include "spdlog/spdlog.h"

#ifndef NEDITOR
#   include "Editor/Editor.hpp"
#endif

#include "Rendering.hpp"
#include "Rendering/All.hpp"
#include "DirectxBackend.hpp"

#include "Timer.hpp"
#include "Main/Time.hpp"
#include "Input.hpp"
#include "Engine.hpp"
#include "ECS/ECS.hpp"
#include "FreeCamera.hpp"
#include "Editor/Gizmo.hpp"
#include "Rendering/Line2D.hpp"
#include "Extern/stb_image.h"
#include "Structures/LinkedList.hpp"
#include "CE_Common.hpp"
#include "Serializer.hpp"
#include "AngleCulling.hpp",
#include "Structures/HString.hpp"
// #include "Scripting/ScriptingEngine.hpp"

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
    SDL_Surface* LoadLogo();
    XMMATRIX matrix;
    Entity* firstEntity;
}

void Engine::AddEndOfFrameEvent(Action action) { EndOfFrameEvents.Add(action);}
void Engine::AddWindowScaleEvent(FunctionAction<void, int, int>::Type act) { ScreenScaleChanged.Bind(act); }

glm::ivec2 Engine::GetWindowScale() {
    glm::ivec2 result;
    SDL_GetWindowSize(window, &result.x, &result.y);
    return result;
}

int Engine::Width() { return GetWindowScale().x; } int Engine::Height() { return GetWindowScale().y; };

glm::ivec2 Engine::GetMainMonitorScale()
{
	SDL_DisplayMode DM;
	SDL_GetCurrentDisplayMode(0, &DM);
	return glm::ivec2(DM.w, DM.h);
}

HWND Engine::GetHWND() { return hwnd; }

SDL_Window* Engine::GetWindow() { return window; };

#ifndef NEDITOR
void Engine::MainWindow()
{
	ImGui::Begin("Settings");

    ImGui::Text(ICON_FA_SMILE "vay be!");

    static float fps = 0;
    if (int(Time::GetTimeSinceStartup()) % 2 > 0)
    {
        fps = 1 / Time::GetDeltaTime();
    }

	ImGui::Text(std::to_string(fps).c_str(), " fps");

	if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_Bullet)) freeCamera->EditorUpdate();

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
    matrix = XMMatrixIdentity() * XMMatrixTranslation(0, 5, 0);
	freeCamera = new FreeCamera(90, Width() / Height(), 0.1f, 9000.0f);

	Renderer3D::Initialize(freeCamera);	Gizmo::Initialize(freeCamera);
#ifndef NEDITOR
    Editor::Initialize(window, DirectxBackend::GetDevice(), DirectxBackend::GetDeviceContext());
    Editor::DarkTheme();
    Editor::AddOnEditor(MainWindow);
#endif
	SceneManager::LoadNewScene();

	firstEntity = new Entity();
	Mesh* mesh = MeshLoader::LoadMesh("Models/sponza.obj");
	MeshRenderer* renderer = new MeshRenderer(mesh);
	Renderer3D::AddMeshRenderer(renderer);
	SceneManager::GetCurrentScene()->AddEntity(firstEntity);
	renderer->SetEntity(firstEntity);
	firstEntity->AddComponent((ECS::Component*)renderer);
}

void Engine::WindowSizeChanged()
{
    WindowSize windowSize = DirectxBackend::GetWindowSize();
#ifndef NEDITOR
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

// https://wiki.libsdl.org/SDL_CreateRGBSurfaceFrom
SDL_Surface* Engine::LoadLogo()
{
    int req_format = STBI_rgb_alpha;
    int width, height, orig_format;
    unsigned char* data = stbi_load("Textures/logo.png", &width, &height, &orig_format, req_format);
    if (data == NULL) { SDL_Log("Loading logo failed: %s", stbi_failure_reason()); }
    return SDL_CreateRGBSurfaceFrom((void*)data, width, height, 32, 4 * width, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
}


#ifndef AXGLOBALCONST
#	if _MSC_VER
#		define AXGLOBALCONST extern const __declspec(selectany)
#	elif defined(__GNUC__) && !defined(__MINGW32__)
#		define AXGLOBALCONST extern const __attribute__((weak))
#	endif
#endif


__declspec(dllexport) void Engine::Start()
{
    //ScriptingEngine::Initialize();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        printf("Error: %s\n", SDL_GetError());
        assert(1, "stl initialization failed");
    }
#ifndef NEDITOR
	const SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
#else
    const SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_BORDERLESS | SDL_WINDOW_FULLSCREEN);
#endif

    SDL_DisplayMode DM;
    SDL_GetCurrentDisplayMode(0, &DM);

	window = SDL_CreateWindow("Castle Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, DM.w, DM.h, window_flags);

    // The icon is attached to the window pointer
    SDL_SetWindowIcon(window, LoadLogo());

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
            done |= event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED)
            {
                // Release all outstanding references to the swap chain's buffers before resizing.
                DirectxBackend::CreateRenderTarget(window);
                WindowSizeChanged();
            }
        }

        // Gizmo::Begin(Engine::GetWindowScale(), Input::GetWindowMousePos(), freeCamera->GetProjection(), freeCamera->GetView());
		// 
		// Gizmo::Manipulate(&firstEntity->transform->GetMatrixRef(), 
		// 				  &firstEntity->transform->position,
		// 			      &firstEntity->transform->quaternion,
		// 				  &firstEntity->transform->scale);
		// 
		// firstEntity->transform->SetQuaternion(firstEntity->transform->quaternion, true);

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