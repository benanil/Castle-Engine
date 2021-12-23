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
#include <SDL.h>
#include <SDL_syswm.h>
#include "Editor/Editor.hpp"
#include "Engine.hpp"
#include <format>
#include <string>
#include "Rendering/Mesh.hpp"
#include "Rendering/Texture.hpp"
#include "Rendering/Shader.hpp"
#include "Rendering/Pipeline.hpp"
#include "ECS/ECS.hpp"
#include "Transform.hpp"
#include "FreeCamera.hpp"

using namespace ECS;

//Global Declarations - Interfaces//
IDXGISwapChain* SwapChain;
DXDevice* Device;
DXDeviceContext* DeviceContext;

// pipeline
DXRenderTargetView* renderTargetView;
DXDepthStencilView* depthStencilView;
DXTexture2D* depthStencilBuffer;
DXBuffer* constantBuffer;
DXRasterizerState* rasterizerState;
DXInputLayout* vertLayout;

// constant buffer
struct cbPerObject
{
    XMMATRIX  MVP;
	XMMATRIX  Model;
} cbPerObj;

//Global Declarations - Others//
SDL_Window* window;
LPCTSTR WndClassName = L"firstwindow";
HWND hwnd = NULL;
HRESULT hr;

const int Width = 1000;
const int Height = 800;

//Function Prototypes//
bool InitializeDirect3d11App();
void CleanupRenderTarget();
void CreateRenderTarget();
void CleanUp();
bool InitScene();
void UpdateScene();
void DrawScene();

Mesh* mesh;
Entity* firstEntity;
Texture* firstTexture;
FreeCamera* freeCamera;
Shader* shader;
// Pipeline* ScreenPipeline;
RenderTexture* renderTexture;

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
        std::string description = std::string(message) + std::string(file) + " at line: " + std::to_string(line);
        SDL_ShowSimpleMessageBox(SDL_MessageBoxFlags::SDL_MESSAGEBOX_ERROR, "DX ERROR!", description.c_str(), window);
        assert(1);
    }
}

DXDevice* Engine::GetDevice() { return Device; }
DXDeviceContext* Engine::GetDeviceContext() { return DeviceContext; }
SDL_Window* Engine::GetWindow() { return window; };

void MainWindow()
{
    ImGui::Begin("SA IMGUI");

	firstEntity->transform->OnEditor();
	freeCamera->EditorUpdate();

	ImGui::Image(renderTexture->GetShaderResourceView(), {50, 50});

    ImGui::End();

    ImGui::Begin("TestWindow");
    for (size_t i = 0; i < 10; i++)
    {
        ImGui::BulletText("sa ImGui::Naber_Moruk");
    }
    ImGui::End();

    Editor::GameViewWindow::Draw();
}

void RenderTextureSizeChanged()
{
    Editor::GameViewWindow::SetTexture(renderTexture->textureView);
}

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

    // init imgui
    Editor::Initialize(window, Device, DeviceContext);
    Editor::DarkTheme();

    Editor::AddOnEditor(MainWindow);

	SceneManager::LoadNewScene();
	
	firstEntity = new Entity();
	SceneManager::GetCurrentScene()->AddEntity(firstEntity);
		
	firstEntity->transform->SetEulerDegree({90, 90, 0});

	freeCamera = new FreeCamera(90, Width / Height, 0.1f, 3000.0f);
	
	cbPerObj.MVP = XMMatrixTranspose(freeCamera->ViewProjection);
	cbPerObj.Model = XMMatrixTranspose(XMMatrixIdentity());

	// Shader* screenShader = new Shader(L"PostProcessing.hlsl", L"PostProcessing.hlsl");
	// ScreenPipeline = new Pipeline(screenShader, 500, 500);

    renderTexture = new RenderTexture(Width, Height, true);
    RenderTextureSizeChanged();
    renderTexture->OnSizeChanged.Add(RenderTextureSizeChanged);

	// Main loop
    bool done = false;
    while (!done)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
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

		SceneManager::GetCurrentScene()->Update();

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
    depthDesc.SampleDesc.Count = 1;
    depthDesc.SampleDesc.Quality = 0;
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

    //Create our Render Target
    Device->CreateRenderTargetView(BackBuffer, NULL, &renderTargetView);
    BackBuffer->Release();

    //Set our Render Target
    DeviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

	if(freeCamera)
	freeCamera->UpdateProjection((float)depthDesc.Width / depthDesc.Height);

    if (renderTexture)
    renderTexture->Invalidate(depthDesc.Width, depthDesc.Height);

	//Update the Viewport
	DX_CREATE(D3D11_VIEWPORT, viewport)
	
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = depthDesc.Width ;
	viewport.Height = depthDesc.Height;
	
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	
	DeviceContext->RSSetViewports(1, &viewport);
    
    static bool firstTime = true;

    if (!firstTime)
    {
        ImGuiViewport* imViewport = ImGui::GetMainViewport();
        imViewport->Size.x = depthDesc.Width;
        imViewport->Size.x = depthDesc.Height;
    }
    
    firstTime = false;
}

void CleanupRenderTarget()
{
	DX_RELEASE(renderTargetView)
	DX_RELEASE(depthStencilBuffer) 
	DX_RELEASE(depthStencilView)
}

bool InitializeDirect3d11App()
{
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
	DX_CREATE(DXGI_SWAP_CHAIN_DESC, swapChainDesc)

    swapChainDesc.BufferDesc = bufferDesc;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 1;
    swapChainDesc.OutputWindow = hwnd;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    //Create our SwapChain
    hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, NULL,
        D3D11_SDK_VERSION, &swapChainDesc, &SwapChain, &Device, NULL, &DeviceContext);

    CreateRenderTarget();
    return true;
}

void CleanUp()
{
    //Release the COM Objects we created
    Editor::Clear();
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
	shader = new Shader(L"First.hlsl", L"First.hlsl");

	//Set Vertex and Pixel Shaders
    DeviceContext->VSSetShader(shader->VS, 0, 0);
    DeviceContext->PSSetShader(shader->PS, 0, 0);

    // Create Constant Buffer

	DX_CREATE(D3D11_BUFFER_DESC, cbDesc);
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    cbDesc.ByteWidth = sizeof(cbPerObject);
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = 0;
    cbDesc.MiscFlags = 0;

    Device->CreateBuffer(&cbDesc, NULL, &constantBuffer);
	
    DeviceContext->UpdateSubresource(constantBuffer, 0, NULL, &cbPerObj, 0, 0);
    DeviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);

    //Create the Input Layout
	
    //Set the Input Layout
	vertLayout = Vertex::GetLayout(shader->VS_Buffer);
    DeviceContext->IASetInputLayout(vertLayout);

    //Set Primitive Topology
    DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Create Rasterizer 
	DX_CREATE(D3D11_RASTERIZER_DESC, rasterizerDesc)
    rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
    rasterizerDesc.CullMode = D3D11_CULL_BACK;

    DX_CHECK(
        Device->CreateRasterizerState(&rasterizerDesc, &rasterizerState), "rasterizer creation failed"
    );

    DeviceContext->RSSetState(NULL);

    //Create the Viewport
	DX_CREATE(D3D11_VIEWPORT, viewport);

    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = Width;
    viewport.Height = Height;

    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    //Set the Viewport
    DeviceContext->RSSetViewports(1, &viewport);

	firstTexture = new Texture(L"Textures/map_Base_Colorenyeni.png");

    mesh = MeshLoader::LoadMesh("Models/map.fbx");

    return true;
}

void UpdateScene()
{
	freeCamera->Update(0.00001f);
}

void DrawScene()
{
    //Clear our backbuffer
    float bgColor[4] = { .2f, .2f, .2f, 1.0f };
    DeviceContext->ClearRenderTargetView(renderTargetView, bgColor);
    DeviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    // 
    // //Draw first cube
	//mesh->Draw(DeviceContext);

    const auto MVP = firstEntity->transform->GetMatrix() * freeCamera->ViewProjection;
    cbPerObj.MVP = XMMatrixTranspose(MVP);
	cbPerObj.Model = XMMatrixTranspose(firstEntity->transform->GetMatrix());
	DeviceContext->UpdateSubresource(constantBuffer, 0, NULL, &cbPerObj, 0, 0);
    DeviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);
	
    firstTexture->Bind(DeviceContext);
    renderTexture->SetAsRendererTarget();
    renderTexture->ClearRenderTarget(bgColor);

    mesh->Draw(DeviceContext);

    // set default render buffers again
    DeviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

	// Draw Imgui
    Editor::Render();

    //Present the backbuffer to the screen
    SwapChain->Present(1, 0);
}
