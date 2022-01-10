//Include and link appropriate libraries and headers//
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")

#include <windows.h>
#include <d3dx11.h>
#include "helper.hpp"
#include <D3DX10.h>
#include <cassert>

#include <algorithm>
#include <utility>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE 

#include <glm/glm.hpp>
#include <array>
#include <vector>
#undef SDL_HAS_VULKAN 
#include <SDL_syswm.h>
#ifndef NEDITOR
#   include "Editor/Editor.hpp"
#endif
#include "Engine.hpp"
#include <format>
#include <string>
#include "Rendering/Mesh.hpp"
#include "Rendering/Texture.hpp"
#include "Rendering/Shader.hpp"
#include "Rendering/Pipeline.hpp"
#include "Rendering/Skybox.hpp"
#include "Rendering/Terrain.hpp"
#include "ECS/ECS.hpp"
#include "Transform.hpp"
#include "FreeCamera.hpp"
#include <map>
#include <iostream>

using namespace ECS;

//Function Prototypes//
bool InitializeDirect3d11App();
void CleanupRenderTarget();
void CreateRenderTarget();
void CleanUp();
bool InitScene();
void UpdateScene();
void DrawScene();

namespace 
{
//Global Declarations - Interfaces//
IDXGISwapChain* SwapChain;
DXDevice* Device;
DXDeviceContext* DeviceContext;

// pipeline
DXRenderTargetView* renderTargetView;
DXDepthStencilView* depthStencilView;
DXTexture2D* depthStencilBuffer;
DXRasterizerState* rasterizerState;
DXRasterizerState* WireframeRasterizerState;

DXInputLayout* vertLayout;
D3D11_VIEWPORT ViewPort;

// constant buffer
cbGlobal cbGlobalData;
cbPerObject cbPerObj;

DXBuffer* constantBuffer;
DXBuffer* uniformGlobalBuffer;

//Global Declarations - Others//
SDL_Window* window;
LPCTSTR WndClassName = L"firstwindow";
HWND hwnd = NULL;
HRESULT hr;

const int Width = 1000;
const int Height = 800;

MeshRenderer* mesh;
Entity* firstEntity;
FreeCamera* freeCamera;
Shader* shader;
// Pipeline* ScreenPipeline;
RenderTexture* renderTexture;
Skybox* skybox;

Event EndOfFrameEvents;

UINT MSAASamples;

std::map<int, bool> keyboard;
std::map<int, bool> mouse;
SDL_Cursor* cursor;
float TimeSinceStartup;
float DeltaTime;
}

// ---TIME--
float Engine::GetDeltaTime() { return DeltaTime; }
float Engine::GetTimeSinceStartup() { return TimeSinceStartup; }

// ---INPUT---
bool Engine::GetKeyDown(int keycode) LAMBDAR(keyboard[keycode])
bool Engine::GetKeyUp(int keycode) LAMBDAR(!keyboard[keycode])

bool Engine::GetMouseButtonDown(int buttonName) LAMBDAR(mouse[buttonName])
bool Engine::GetMouseButtonUp(int buttonName) LAMBDAR(mouse[buttonName])

void Engine::SetCursor(SDL_Cursor* _cursor) LAMBDA(cursor = _cursor)
SDL_Cursor* Engine::GetCursor() LAMBDAR(cursor);

void Engine::AddEndOfFrameEvent(const Action& act)
{
    EndOfFrameEvents.Add(act);
}

void Engine::DirectXCheck(const HRESULT& hr, const int line, const char* file)
{
    if (FAILED(hr))
    {
        std::string description = std::string("UNKNOWN ERROR!\n") + std::string(file) + " at line: " + std::to_string(line);
        SDL_ShowSimpleMessageBox(SDL_MessageBoxFlags::SDL_MESSAGEBOX_ERROR, "DX ERROR!", description.c_str(), window);
        assert(1);
    }
}

void Engine::DirectXCheck(const HRESULT& hr, const char* message, const int line, const char* file)
{
    if (FAILED(hr))
    {
        std::cout << "hresult is: " << hr << std::endl;
        std::string description = std::string(message) + std::string(file) + " at line: " + std::to_string(line);
        SDL_ShowSimpleMessageBox(SDL_MessageBoxFlags::SDL_MESSAGEBOX_ERROR, "DX ERROR!", description.c_str(), window);
        assert(1);
    }
}

DXDevice* Engine::GetDevice() { return Device; }
DXDeviceContext* Engine::GetDeviceContext() { return DeviceContext; }
SDL_Window* Engine::GetWindow() { return window; };

#ifndef NEDITOR
void MainWindow()
{
    ImGui::Begin("Settings");

    if (ImGui::CollapsingHeader("Camera")) freeCamera->EditorUpdate();

    if (ImGui::CollapsingHeader("Lighting"))
    {
        ImGui::ColorEdit3("ambient Color", &cbGlobalData.ambientColor.x);
        ImGui::ColorEdit4("sun Color", &cbGlobalData.sunColor.x);
        ImGui::DragFloat("sun angle", &cbGlobalData.sunAngle);
        ImGui::DragFloat("ambientStrength", &cbGlobalData.ambientStength, 0.01f);
    }

	if (ImGui::CollapsingHeader("Terrain"))
	{
		Terrain::OnEditor();
	}

    ImGui::End();

    Editor::GameViewWindow::Draw();

    if (ECS::SceneManager::GetCurrentScene())
    {
        ECS::SceneManager::GetCurrentScene()->UpdateEditor();
    }
}
#endif

int main(int, char**)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    const SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    window = SDL_CreateWindow("Castle Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Width, Height, window_flags);
    SDL_SysWMinfo wmInfo{};
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(window, &wmInfo);

    hwnd = (HWND)wmInfo.info.win.window;

    InitializeDirect3d11App();    //Initialize Direct3D

    InitScene();    //Initialize our scene

#ifndef NEDITOR
    // init imgui
    Editor::Initialize(window, Device, DeviceContext);
    Editor::DarkTheme();

    Editor::AddOnEditor(MainWindow);
#endif

    SceneManager::LoadNewScene();

    firstEntity = new Entity();
    SceneManager::GetCurrentScene()->AddEntity(firstEntity);

    firstEntity->AddComponent((ECS::Component*)mesh);

    freeCamera = new FreeCamera(90, Width / Height, 0.1f, 90'000.0f);

    cbPerObj.MVP = XMMatrixTranspose(freeCamera->ViewProjection);
    cbPerObj.Model = XMMatrixTranspose(XMMatrixIdentity());

#ifndef NEDITOR
    renderTexture = new RenderTexture(Width, Height, MSAASamples, true);
    Editor::GameViewWindow::GetData().texture = renderTexture->textureView;

    renderTexture->OnSizeChanged.Add([](DXShaderResourceView* texture)
        {
            Editor::GameViewWindow::GetData().texture = texture;
        });

    auto& viewWindowdata = Editor::GameViewWindow::GetData();

    viewWindowdata.OnScaleChanged.Add([](const float& w, const float& h)
        {
            freeCamera->aspectRatio = w / h;
            freeCamera->UpdateProjection(freeCamera->aspectRatio);
            renderTexture->Invalidate((int)w, (int)h);
        });
#endif

    // Main loop
    bool done = false;
    float lastTime = 0;
    
    while (!done)
    {
        SDL_Event event;

		DeltaTime = (lastTime - (float)SDL_GetTicks()) / 1000.0f;
		lastTime = SDL_GetTicks();
		TimeSinceStartup += DeltaTime;

		while (SDL_PollEvent(&event))
        {
#ifndef NEDITOR
            ImGui_ImplSDL2_ProcessEvent(&event);
#endif
            SceneManager::GetCurrentScene()->ProceedEvent(&event);
            freeCamera->Update(DeltaTime);

            switch (event.type)
            {
            case SDL_KEYDOWN: keyboard[event.key.keysym.sym] = true; break;
            case SDL_KEYUP:   keyboard[event.key.keysym.sym] = false; break;
            }

            switch (event.button.type)
            {
            case SDL_MOUSEBUTTONDOWN: mouse[event.button.button] = true; break;
            case SDL_MOUSEBUTTONUP:   mouse[event.button.button] = false; break;
            }

            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED && event.window.windowID == SDL_GetWindowID(window))
            {
                // Release all outstanding references to the swap chain's buffers before resizing.
                CleanupRenderTarget();
                SwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
                CreateRenderTarget();
            }
        }

        SceneManager::GetCurrentScene()->Update(DeltaTime);
        UpdateScene();
        DrawScene();
    }

    CleanUp();

    return 0;
}

void CreateRenderTarget()
{
    D3D11_TEXTURE2D_DESC depthDesc{};
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = MSAASamples;
    depthDesc.SampleDesc.Quality = MSAASamples;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthDesc.CPUAccessFlags = 0;
    depthDesc.MiscFlags = 0;

    SDL_GetWindowSize(window, reinterpret_cast<int*>(&depthDesc.Width), reinterpret_cast<int*>(&depthDesc.Height));

    Device->CreateTexture2D(&depthDesc, NULL, &depthStencilBuffer);
    Device->CreateDepthStencilView(depthStencilBuffer, NULL, &depthStencilView);

    //Create our BackBuffer
    DXTexture2D* BackBuffer;
    SwapChain->GetBuffer(0, IID_PPV_ARGS(&BackBuffer));

    DX_CREATE(D3D11_RENDER_TARGET_VIEW_DESC, rtvDesc);

    D3D11_TEXTURE2D_DESC backDesc;
    BackBuffer->GetDesc(&backDesc);

    rtvDesc.Format = backDesc.Format;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;

    //Create our Render Target
    Device->CreateRenderTargetView(BackBuffer, &rtvDesc, &renderTargetView);
    BackBuffer->Release();

    //Set our Render Target
    DeviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

    if (freeCamera)
    {
        freeCamera->UpdateProjection((float)depthDesc.Width / depthDesc.Height);
    }

    //Update the Viewport
    memset(&ViewPort, 0, sizeof(D3D11_VIEWPORT));

    ViewPort.TopLeftX = 0;
    ViewPort.TopLeftY = 0;
    ViewPort.Width = depthDesc.Width;
    ViewPort.Height = depthDesc.Height;

    ViewPort.MinDepth = 0.0f;
    ViewPort.MaxDepth = 1.0f;

    DeviceContext->RSSetViewports(1, &ViewPort);

#ifndef NEDITOR
    static bool firstTime = true;
    if (!firstTime)
    {
        ImGuiViewport* imViewport = ImGui::GetMainViewport();
        imViewport->Size.x = depthDesc.Width;
        imViewport->Size.x = depthDesc.Height;
    }
    firstTime = false;
#endif
}


void CleanupRenderTarget()
{
    DX_RELEASE(renderTargetView)
    DX_RELEASE(depthStencilBuffer)
    DX_RELEASE(depthStencilView)
}

bool InitializeDirect3d11App()
{
    UINT creationFlags = D3D11_CREATE_DEVICE_DEBUG;

    DX_CHECK(
    D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, creationFlags, NULL, NULL, D3D11_SDK_VERSION, &Device, NULL, &DeviceContext),
    "Device Creation Failed!")

#ifndef NEDITOR
    MSAASamples = 2;
#else
    // Determine maximum supported MSAA level.
    for (MSAASamples = 16; MSAASamples > 1; MSAASamples >>= 1) {
        UINT colorQualityLevels;
        UINT depthStencilQualityLevels;
        Device->CheckMultisampleQualityLevels(DXGI_FORMAT_R16G16B16A16_FLOAT, MSAASamples, &colorQualityLevels);
        Device->CheckMultisampleQualityLevels(DXGI_FORMAT_D24_UNORM_S8_UINT, MSAASamples, &depthStencilQualityLevels);
        if (colorQualityLevels > 0 && depthStencilQualityLevels > 0) {
            break;
        }
    }
#endif

    IDXGIDevice* pDXGIDevice = nullptr;
    hr = Device->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDXGIDevice);

    IDXGIAdapter* pDXGIAdapter = nullptr;
    hr = pDXGIDevice->GetAdapter(&pDXGIAdapter);

    IDXGIFactory* pIDXGIFactory = nullptr;
    pDXGIAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&pIDXGIFactory);

    //Describe our Buffer
    DX_CREATE(DXGI_MODE_DESC, bufferDesc);

    bufferDesc.Width = 0;
    bufferDesc.Height = 0;
    bufferDesc.RefreshRate.Numerator = 60;
    bufferDesc.RefreshRate.Denominator = 1;
    bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    //Describe our SwapChain
    DX_CREATE(DXGI_SWAP_CHAIN_DESC, swapChainDesc);

    swapChainDesc.BufferDesc = bufferDesc;
    swapChainDesc.SampleDesc.Count = MSAASamples;
    swapChainDesc.SampleDesc.Quality = MSAASamples;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 1;
    swapChainDesc.OutputWindow = hwnd;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    DX_CHECK(
    pIDXGIFactory->CreateSwapChain(Device, &swapChainDesc, &SwapChain), "SwapChain Creation Failed!")

    std::cout << "initialized" << std::endl;

    CreateRenderTarget();
    return true;
}

void CleanUp()
{
    //Release the COM Objects we created
#ifndef NEDITOR
    Editor::Clear();
#endif
    mesh->Dispose();
    SwapChain->Release();
    Device->Release();
    DeviceContext->Release();
    renderTargetView->Release();
    depthStencilView->Release();
    depthStencilBuffer->Release();
    constantBuffer->Release();

    SDL_DestroyWindow(window);
    SDL_Quit();
}


bool InitScene()
{
    //Compile Shaders from shader file
    shader = new Shader("First.hlsl\0", "First.hlsl\0");

    //Set Vertex and Pixel Shaders
    shader->Bind();

    // create uniform constant buffer
    cbGlobalData.ambientColor = glm::vec3(0.8f, 0.8f, 0.65f);
    cbGlobalData.sunColor = glm::vec3(1.0f, 1.0f, 1.0f);
    cbGlobalData.ambientStength = .13f;
    cbGlobalData.sunAngle = 120;

    DX_CREATE(D3D11_BUFFER_DESC, cbUniformDesc);
    cbUniformDesc.Usage = D3D11_USAGE_DEFAULT;
    cbUniformDesc.ByteWidth = sizeof(cbGlobal);
    cbUniformDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbUniformDesc.CPUAccessFlags = 0;
    cbUniformDesc.MiscFlags = 0;

    Device->CreateBuffer(&cbUniformDesc, NULL, &uniformGlobalBuffer);

    // Create Constant Buffer

    DX_CREATE(D3D11_BUFFER_DESC, cbDesc);
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    cbDesc.ByteWidth = sizeof(cbPerObject);
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = 0;
    cbDesc.MiscFlags = 0;

    Device->CreateBuffer(&cbDesc, NULL, &constantBuffer);

    // bind constant data
    DeviceContext->UpdateSubresource(constantBuffer, 0, NULL, &cbPerObj, 0, 0);
    DeviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);

    // bind Uniform data
    DeviceContext->UpdateSubresource(uniformGlobalBuffer, 0, NULL, &cbGlobalData, 0, 0);
    DeviceContext->PSSetConstantBuffers(2, 1, &uniformGlobalBuffer);

    //Create the Input Layout & Set the Input Layout
    vertLayout = Vertex::GetLayout(shader->VS_Buffer);
    DeviceContext->IASetInputLayout(vertLayout);

    //Set Primitive Topology
    DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Create Rasterizer 
	DX_CREATE(D3D11_RASTERIZER_DESC, rasterizerDesc);
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_BACK;
    rasterizerDesc.MultisampleEnable = true;

    DX_CHECK(
	Device->CreateRasterizerState(&rasterizerDesc, &rasterizerState), "rasterizer creation failed");

	memset(&rasterizerDesc, 0, sizeof(D3D11_RASTERIZER_DESC));
	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.MultisampleEnable = true;

	DeviceContext->RSSetState(rasterizerState);

	DX_CHECK(
		Device->CreateRasterizerState(&rasterizerDesc, &WireframeRasterizerState), "rasterizer creation failed");

    //Create the Viewport
    memset(&ViewPort, 0, sizeof(D3D11_VIEWPORT));

    ViewPort.TopLeftX = 0;
    ViewPort.TopLeftY = 0;
    ViewPort.Width = Width;
    ViewPort.Height = Height;
    ViewPort.MinDepth = 0.0f;
    ViewPort.MaxDepth = 1.0f;

    //Set the Viewport
    DeviceContext->RSSetViewports(1, &ViewPort);

    mesh = MeshLoader::LoadMesh("Models/sponza.obj");
    mesh->SetEntity(firstEntity);

	skybox = new Skybox(10, 10, MSAASamples);
    std::cout << "skybox created" << std::endl;

	Terrain::Initialize();
    
    std::cout << "Terrain created" << std::endl;

    return true;
}

void UpdateScene()
{
    freeCamera->Update(0.00001f);

    const auto MVP = firstEntity->transform->GetMatrix() * freeCamera->ViewProjection;
    cbPerObj.MVP = XMMatrixTranspose(MVP);
    cbPerObj.Model = XMMatrixTranspose(firstEntity->transform->GetMatrix());

    cbGlobalData.viewPos = freeCamera->transform.position;

    DeviceContext->UpdateSubresource(constantBuffer, 0, NULL, &cbPerObj, 0, 0);
    DeviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);

	// use additionalData  as time since startup
	cbGlobalData.additionalData = TimeSinceStartup;

	DeviceContext->UpdateSubresource(uniformGlobalBuffer, 0, NULL, &cbGlobalData, 0, 0);
    DeviceContext->PSSetConstantBuffers(2, 1, &uniformGlobalBuffer);
}

void DrawTerrain()
{
    Terrain::BindShader();

    cbPerObj.MVP = XMMatrixTranspose(XMMatrixTranslation(-000, 0, -1100) * XMMatrixScaling(7, 7, 7) * freeCamera->ViewProjection);

    DeviceContext->UpdateSubresource(constantBuffer, 0, NULL, &cbPerObj, 0, 0);
    DeviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);

    cbGlobalData.additionalData = Terrain::GetTextureScale();
    DeviceContext->UpdateSubresource(uniformGlobalBuffer, 0, NULL, &cbGlobalData, 0, 0);

    DeviceContext->PSSetConstantBuffers(2, 1, &uniformGlobalBuffer);
	DeviceContext->RSSetState(rasterizerState);

    Terrain::Draw();
}

void DrawScene()
{
#ifndef NEDITOR
    float oldViewPortW = ViewPort.Width, oldViewPortH = ViewPort.Height;

    auto& gameVindowdata = Editor::GameViewWindow::GetData();
    ViewPort.Width = gameVindowdata.WindowScale.x;
    ViewPort.Height = gameVindowdata.WindowScale.y;

    DeviceContext->RSSetViewports(1, &ViewPort);
    renderTexture->SetBlendState();
#endif

    //Clear our backbuffer
    float bgColor[4] = { .4, .4, .7, 1.0f };
    DeviceContext->ClearRenderTargetView(renderTargetView, bgColor);    
    DeviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

#ifdef NEDITOR
    // rendering to back buffer here
    mesh->Draw(DeviceContext);
#else
    // rendering to texture Here
    renderTexture->ClearRenderTarget(bgColor);
    renderTexture->SetAsRendererTarget();
#endif
	DeviceContext->RSSetState(rasterizerState);

	mesh->Draw(DeviceContext);

	skybox->Draw(cbPerObj, DeviceContext, constantBuffer, freeCamera);

    DrawTerrain();
	
	// set default render buffers again
	DeviceContext->OMSetDepthStencilState(nullptr, 0);
	DeviceContext->RSSetState(rasterizerState);

	DeviceContext->IASetInputLayout(vertLayout);
    DeviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

#ifndef NEDITOR
    ViewPort.Width = oldViewPortW; ViewPort.Height = oldViewPortH;
    DeviceContext->RSSetViewports(1, &ViewPort);
#endif

	shader->Bind();

#ifndef NEDITOR
	// Draw Imgui
    Editor::Render();
#endif

    //Present the backbuffer to the screen
    SwapChain->Present(1, 0);

    EndOfFrameEvents.Invoke();
    EndOfFrameEvents.Clear();
}