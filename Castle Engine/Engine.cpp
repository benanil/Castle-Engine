//Include and link appropriate libraries and headers//
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")

#include <windows.h>
#include <d3dx11.h>
#include "helper.hpp"
#include <D3DX10.h>
#include <xnamath.h>
#include <cassert>

#include <algorithm>
#include <utility>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE 

#include <glm/glm.hpp>
#include <array>
#include <vector>
#include <SDL.h>
#include <SDL_syswm.h>
#include "Editor/Editor.hpp"
#include "Engine.hpp"
#include <format>
#include <string>
#include "Rendering/Mesh.hpp"

//Global Declarations - Interfaces//
IDXGISwapChain* SwapChain;
DXDevice* d3d11Device;
DXDeviceContext* d3d11DevCon;
DXRenderTargetView* renderTargetView;

ID3D11DepthStencilView* depthStencilView;
DXTexture2D* depthStencilBuffer;

DXBuffer* constantBuffer;

ID3D11RasterizerState* wireFrame;

DXTextureView* cubeTextureView;
DXTexSampler* cubeTexSampler;

///
XMMATRIX cube1World;

XMMATRIX Rotation;
XMMATRIX Scale;
XMMATRIX Translation;
float rot = 0.01f;

// constant buffer
XMMATRIX ViewProjection;
XMMATRIX camView;
XMMATRIX camProjection;

XMVECTOR camPosition;
XMVECTOR camUp;
XMVECTOR camTarget;

struct cbPerObject
{
    XMMATRIX  MVP;
	XMMATRIX  Model;
} cbPerObj;

ID3D11VertexShader* VS;
ID3D11PixelShader* PS;
ID3D10Blob* VS_Buffer;
ID3D10Blob* PS_Buffer;
ID3D11InputLayout* vertLayout;

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

DXDevice* Engine::GetDevice() { return d3d11Device; }
DXDeviceContext* Engine::GetDeviceContext() { return d3d11DevCon; }
const SDL_Window* Engine::GetWindow() { return window; };

glm::vec3 axis { 1.57, 1.57, 0};
glm::vec3 cameraPos {500, 140, 0};
float angle = -1.57f;

void MainWindow()
{
    ImGui::Begin("SA IMGUI");

    static char gir[128];

    ImGui::InputText("gir", gir, 128);

	ImGui::DragFloat3("axis", &axis.x, 0.1f);
	ImGui::DragFloat3("cameraPos", &cameraPos.x, 0.1f);
    ImGui::DragFloat("angle", &angle, 0.1f);
    
    ImGui::End();

    ImGui::Render();
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
    Editor::Initialize(window, d3d11Device, d3d11DevCon);
    Editor::DarkTheme();

    Editor::AddOnEditor(MainWindow);

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

    d3d11Device->CreateTexture2D(&depthDesc, NULL, &depthStencilBuffer);
    d3d11Device->CreateDepthStencilView(depthStencilBuffer, NULL, &depthStencilView);

    //Create our BackBuffer
    ID3D11Texture2D* BackBuffer;
    hr = SwapChain->GetBuffer(0, IID_PPV_ARGS(&BackBuffer));

    //Create our Render Target
    hr = d3d11Device->CreateRenderTargetView(BackBuffer, NULL, &renderTargetView);
    BackBuffer->Release();

    //Set our Render Target
    d3d11DevCon->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

    camProjection = XMMatrixPerspectiveFovLH(0.33f * 3.14f, (float)depthDesc.Width / depthDesc.Height, 1.0f, 1000.0f);
	
	//Update the Viewport
	D3D11_VIEWPORT viewport = DX_CREATE<D3D11_VIEWPORT>();
	
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = depthDesc.Width ;
	viewport.Height = depthDesc.Height;
	
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	
	d3d11DevCon->RSSetViewports(1, &viewport);
}

void CleanupRenderTarget()
{
    if (renderTargetView) { renderTargetView->Release(); renderTargetView = NULL; }
    if (depthStencilBuffer) { depthStencilBuffer->Release(); depthStencilView->Release(); }
}

bool InitializeDirect3d11App()
{
    //Describe our Buffer
    DXGI_MODE_DESC bufferDesc = DX_CREATE<DXGI_MODE_DESC>();

    bufferDesc.Width = 0;
    bufferDesc.Height = 0;
    bufferDesc.RefreshRate.Numerator = 60;
    bufferDesc.RefreshRate.Denominator = 1;
    bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    //Describe our SwapChain
    DXGI_SWAP_CHAIN_DESC swapChainDesc = DX_CREATE<DXGI_SWAP_CHAIN_DESC>();

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
        D3D11_SDK_VERSION, &swapChainDesc, &SwapChain, &d3d11Device, NULL, &d3d11DevCon);

    CreateRenderTarget();
    return true;
}

void CleanUp()
{
    //Release the COM Objects we created
    Editor::Clear();
    mesh->Dispose();
    SwapChain->Release();
    d3d11Device->Release();
    d3d11DevCon->Release();
    renderTargetView->Release();
    depthStencilView->Release();
    depthStencilBuffer->Release();
    constantBuffer->Release();
    VS->Release();
    PS->Release();
    VS_Buffer->Release();
    PS_Buffer->Release();
    vertLayout->Release();

    SDL_DestroyWindow(window);
    SDL_Quit();
}

bool InitScene()
{
    //Compile Shaders from shader file
    DX_CHECK(
        D3DX11CompileFromFile(L"First.hlsl", 0, 0, "VS", "vs_4_0", 0, 0, 0, &VS_Buffer, 0, 0),
        "VertexShader Compiling error! \n"
    );

    DX_CHECK(
        D3DX11CompileFromFile(L"First.hlsl", 0, 0, "PS", "ps_4_0", 0, 0, 0, &PS_Buffer, 0, 0),
        "FragShader Compiling error! \n"
    );

    //Create the Shader Objects
    DX_CHECK(
        d3d11Device->CreateVertexShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), NULL, &VS),
        "VertexShader Compiling error! \n"
    );

    DX_CHECK(
        d3d11Device->CreatePixelShader(PS_Buffer->GetBufferPointer(), PS_Buffer->GetBufferSize(), NULL, &PS),
        "FragShader Compiling error! \n"
    );

    //Set Vertex and Pixel Shaders
    d3d11DevCon->VSSetShader(VS, 0, 0);
    d3d11DevCon->PSSetShader(PS, 0, 0);

    // Create Constant Buffer

    D3D11_BUFFER_DESC cbDesc = DX_CREATE<D3D11_BUFFER_DESC>();
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    cbDesc.ByteWidth = sizeof(cbPerObject);
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = 0;
    cbDesc.MiscFlags = 0;

    d3d11Device->CreateBuffer(&cbDesc, NULL, &constantBuffer);

    camPosition = XMVectorSet(0.0f, 0.0f, -30.0f, -30.0f);
    camTarget = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
    camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    camView = XMMatrixLookAtLH(camPosition, camTarget, camUp);
    camProjection = XMMatrixPerspectiveFovLH(0.4f * 3.14f, (float)Width / Height, 1.0f, 1000.0f);

    ViewProjection = XMMatrixIdentity() * camView * camProjection;
    cbPerObj.MVP = XMMatrixTranspose(ViewProjection);
	cbPerObj.Model          = XMMatrixTranspose(XMMatrixIdentity());
	
    d3d11DevCon->UpdateSubresource(constantBuffer, 0, NULL, &cbPerObj, 0, 0);
    d3d11DevCon->VSSetConstantBuffers(0, 1, &constantBuffer);

    //Create the Input Layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 , D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL"  , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT   , 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numElements = ARRAYSIZE(layout);

    d3d11Device->CreateInputLayout(layout, numElements, VS_Buffer->GetBufferPointer(),
        VS_Buffer->GetBufferSize(), &vertLayout);

    //Set the Input Layout
    d3d11DevCon->IASetInputLayout(vertLayout);

    //Set Primitive Topology
    d3d11DevCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Create Rasterizer 
    D3D11_RASTERIZER_DESC rasterizerDesc = DX_CREATE<D3D11_RASTERIZER_DESC>();
    rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
    rasterizerDesc.CullMode = D3D11_CULL_BACK;

    DX_CHECK(
        d3d11Device->CreateRasterizerState(&rasterizerDesc, &wireFrame), "rasterizer creation failed"
    );

    d3d11DevCon->RSSetState(NULL);

    //Create the Viewport
    D3D11_VIEWPORT viewport = DX_CREATE<D3D11_VIEWPORT>();

    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = Width;
    viewport.Height = Height;

    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    //Set the Viewport
    d3d11DevCon->RSSetViewports(1, &viewport);

    // create texture
	DX_CHECK(
		D3DX11CreateShaderResourceViewFromFile(d3d11Device, L"Textures/map_Base_Colorenyeni.png",
		NULL, NULL, &cubeTextureView, NULL), "Textures/map_Base_Colorenyeni.png Texture Creation Failed!"
	)

    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    hr = d3d11Device->CreateSamplerState(&sampDesc, &cubeTexSampler);

    mesh = MeshLoader::LoadMesh("Models/map.fbx");

    return true;
}

void UpdateScene()
{
    //Keep the cubes rotating
    rot += .0005f;
    if (rot > 6.28f)
        rot = 0.0f;

    camPosition = XMVectorSet(cameraPos.x, cameraPos.y, cameraPos.z, 0.0f);
    camTarget = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
    camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    camView = XMMatrixLookAtLH(camPosition, camTarget, camUp);

    //Reset cube1World
    cube1World = XMMatrixIdentity();

    //Define cube1's world space matrix
    Rotation    = XMMatrixRotationRollPitchYaw(axis.x, axis.y, axis.z);
    Translation = XMMatrixIdentity();

    //Set cube1's world space using the transformations
	cube1World = Translation * Rotation;
}


void DrawScene()
{
    //Clear our backbuffer
    float bgColor[4] = { .2f, .2f, .2f, 1.0f };
    d3d11DevCon->ClearRenderTargetView(renderTargetView, bgColor);
    d3d11DevCon->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    ViewProjection = cube1World * camView * camProjection;
    cbPerObj.MVP = XMMatrixTranspose(ViewProjection);
	cbPerObj.Model = XMMatrixTranspose(cube1World);
	d3d11DevCon->UpdateSubresource(constantBuffer, 0, NULL, &cbPerObj, 0, 0);
    d3d11DevCon->VSSetConstantBuffers(0, 1, &constantBuffer);

    ///////////////**************new**************////////////////////
    d3d11DevCon->PSSetShaderResources(0, 1, &cubeTextureView);
    d3d11DevCon->PSSetSamplers(0, 1, &cubeTexSampler);

    //Draw first cube
	mesh->Draw(d3d11DevCon);

    // Draw Imgui
    Editor::Render();

    //Present the backbuffer to the screen
    SwapChain->Present(0, 0);
}
