
//Include and link appropriate libraries and headers//
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")

#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <D3DX10.h>
#include <xnamath.h>
#include <cassert>

#include <algorithm>
#include <utility>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE 

#include <glm/glm.hpp>
#include "helper.hpp"
#include <array>
#include <vector>
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_dx11.h"
#include <SDL.h>
#include <SDL_syswm.h>

// #include <SDL.h>

//Global Declarations - Interfaces//
IDXGISwapChain* SwapChain;
ID3D11Device* d3d11Device;
ID3D11DeviceContext* d3d11DevCon;
ID3D11RenderTargetView* renderTargetView;

ID3D11DepthStencilView* depthStencilView;
ID3D11Texture2D* depthStencilBuffer;

ID3D11Buffer* triangleVertBuffer;
ID3D11Buffer* indexBuffer;
ID3D11Buffer* constantBuffer;

ID3D11RasterizerState* wireFrame;

///

XMMATRIX cube1World;
XMMATRIX cube2World;

XMMATRIX Rotation;
XMMATRIX Scale;
XMMATRIX Translation;
float rot = 0.01f;


// constant buffer

XMMATRIX MVP;
XMMATRIX World;
XMMATRIX camView;
XMMATRIX camProjection;

XMVECTOR camPosition;
XMVECTOR camUp;
XMVECTOR camTarget;

struct cbPerObject
{
    XMMATRIX  WVP;
} cbPerObj;

ID3D11VertexShader* VS;
ID3D11PixelShader* PS;
ID3D10Blob* VS_Buffer;
ID3D10Blob* PS_Buffer;
ID3D11InputLayout* vertLayout;


//Global Declarations - Others//
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

bool InitializeWindow(HINSTANCE hInstance,
    int ShowWnd,
    int width, int height,
    bool windowed);

static inline void DX_CHECK(HRESULT hr) { if (FAILED(hr)) { MessageBox(hwnd, TEXT("directx error"), TEXT("directx error"), 0);  assert(1); } }
static inline void DX_CHECK(HRESULT hr, const char* message) { if (FAILED(hr)) { MessageBox(hwnd, (const wchar_t*)message, TEXT("directx error"), 0);  assert(1); } }

//Vertex Structure and Vertex Layout (Input Layout)//
struct Vertex    //Overloaded Vertex Structure
{
    glm::vec3 pos;
    glm::vec4 col;

    Vertex() {}
    Vertex(const glm::vec3& _pos, const glm::vec4& color)
        : pos(_pos), col(color)
    {}
};

D3D11_INPUT_ELEMENT_DESC layout[] =
{
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};
UINT numElements = ARRAYSIZE(layout);


SDL_Window* window;

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
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplSDL2_InitForD3D(window);
    ImGui_ImplDX11_Init(d3d11Device, d3d11DevCon);
    ImGui::StyleColorsDark();

    // Main loop
    bool done = false;
    while (!done)
    {
        SDL_Event event;
        while(SDL_PollEvent(&event))
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
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SwapChain->Release();
    d3d11Device->Release();
    d3d11DevCon->Release();
    renderTargetView->Release();
    depthStencilView->Release();
    depthStencilBuffer->Release();
    indexBuffer->Release();
    triangleVertBuffer->Release();
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
        "VertexShader Compiling error"
    );

    DX_CHECK(
        D3DX11CompileFromFile(L"First.hlsl", 0, 0, "PS", "ps_4_0", 0, 0, 0, &PS_Buffer, 0, 0),
        "FragShader Compiling error"
    );

    //Create the Shader Objects
    hr = d3d11Device->CreateVertexShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), NULL, &VS);
    hr = d3d11Device->CreatePixelShader(PS_Buffer->GetBufferPointer(), PS_Buffer->GetBufferSize(), NULL, &PS);

    //Set Vertex and Pixel Shaders
    d3d11DevCon->VSSetShader(VS, 0, 0);
    d3d11DevCon->PSSetShader(PS, 0, 0);

    // create Index Buffer
    std::vector<uint32_t> indices{
        // front face
        0, 1, 2,
        0, 2, 3,
        // back face
        4, 6, 5,
        4, 7, 6,
        // left face
        4, 5, 1,
        4, 1, 0,
        // right face
        3, 2, 6,
        3, 6, 7,
        // top face
        1, 5, 6,
        1, 6, 2,
        // bottom face
        4, 0, 3,
        4, 3, 7
    };

    D3D11_BUFFER_DESC indexDesc = DX_CREATE<D3D11_BUFFER_DESC>();
    indexDesc.Usage = D3D11_USAGE_DEFAULT;
    indexDesc.ByteWidth = sizeof(uint32_t) * indices.size();
    indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexDesc.CPUAccessFlags = 0;
    indexDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = indices.data();
    d3d11Device->CreateBuffer(&indexDesc, &initData, &indexBuffer);
    d3d11DevCon->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Create Constant Buffer

    D3D11_BUFFER_DESC cbDesc = DX_CREATE<D3D11_BUFFER_DESC>();
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    cbDesc.ByteWidth = sizeof(cbPerObject);
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = 0;
    cbDesc.MiscFlags = 0;

    d3d11Device->CreateBuffer(&cbDesc, NULL, &constantBuffer);

    camPosition = XMVectorSet(0.0f, 3.0f, -8.0f, 0.0f);
    camTarget = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    camView = XMMatrixLookAtLH(camPosition, camTarget, camUp);
    camProjection = XMMatrixPerspectiveFovLH(0.4f * 3.14f, (float)Width / Height, 1.0f, 1000.0f);

    World = XMMatrixIdentity();

    MVP = World * camView * camProjection;
    cbPerObj.WVP = XMMatrixTranspose(MVP);

    d3d11DevCon->UpdateSubresource(constantBuffer, 0, NULL, &cbPerObj, 0, 0);
    d3d11DevCon->VSSetConstantBuffers(0, 1, &constantBuffer);

    // Create the vertex buffer
    std::array<Vertex, 8> vertices =
    {
        Vertex({-1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}),
        Vertex({-1.0f, +1.0f, -1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}),
        Vertex({+1.0f, +1.0f, -1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}),
        Vertex({+1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 0.0f, 1.0f}),

        Vertex({-1.0f, -1.0f, +1.0f}, {0.0f, 1.0f, 1.0f, 1.0f}),
        Vertex({-1.0f, +1.0f, +1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}),
        Vertex({+1.0f, +1.0f, +1.0f}, {1.0f, 0.0f, 1.0f, 1.0f}),
        Vertex({+1.0f, -1.0f, +1.0f}, {1.0f, 0.0f, 0.0f, 1.0f})
    };

    D3D11_BUFFER_DESC vertexBufferDesc = DX_CREATE<D3D11_BUFFER_DESC>();

    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(Vertex) * vertices.size();
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA vertexBufferData = DX_CREATE<D3D11_SUBRESOURCE_DATA>();

    vertexBufferData.pSysMem = vertices.data();
    hr = d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &triangleVertBuffer);

    //Set the vertex buffer
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    d3d11DevCon->IASetVertexBuffers(0, 1, &triangleVertBuffer, &stride, &offset);

    //Create the Input Layout
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

    d3d11DevCon->RSSetState(wireFrame);

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

    return true;
}

void UpdateScene()
{
    //Keep the cubes rotating
    rot += .0005f;
    if (rot > 6.28f)
        rot = 0.0f;

    //Reset cube1World
    cube1World = XMMatrixIdentity();

    //Define cube1's world space matrix
    XMVECTOR rotaxis = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    Rotation = XMMatrixRotationAxis(rotaxis, rot);
    Translation = XMMatrixTranslation(0.0f, 0.0f, 4.0f);

    //Set cube1's world space using the transformations
    cube1World = Translation * Rotation;

    //Reset cube2World
    cube2World = XMMatrixIdentity();

    //Define cube2's world space matrix
    Rotation = XMMatrixRotationAxis(rotaxis, -rot);
    Scale = XMMatrixScaling(1.3f, 1.3f, 1.3f);

    //Set cube2's world space matrix
    cube2World = Rotation * Scale;
}

void DrawScene()
{
    //Clear our backbuffer
    float bgColor[4] = { .2f, .2f, .2f, 1.0f };
    d3d11DevCon->ClearRenderTargetView(renderTargetView, bgColor);
    d3d11DevCon->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    MVP = cube1World * camView * camProjection;
    cbPerObj.WVP = XMMatrixTranspose(MVP);
    d3d11DevCon->UpdateSubresource(constantBuffer, 0, NULL, &cbPerObj, 0, 0);
    d3d11DevCon->VSSetConstantBuffers(0, 1, &constantBuffer);

    d3d11DevCon->RSSetState(wireFrame);

    //Draw first cube
    d3d11DevCon->DrawIndexed(36, 0, 0);

    MVP = cube2World * camView * camProjection;
    cbPerObj.WVP = XMMatrixTranspose(MVP);
    d3d11DevCon->UpdateSubresource(constantBuffer, 0, NULL, &cbPerObj, 0, 0);
    d3d11DevCon->VSSetConstantBuffers(0, 1, &constantBuffer);

    d3d11DevCon->RSSetState(NULL);

    //Draw second cube
    d3d11DevCon->DrawIndexed(36, 0, 0);

    // Start the Dear ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("SA IMGUI");

    static char gir[128];

    ImGui::InputText("gir", gir, 128);

    ImGui::End();

    ImGui::Render();

    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    //Present the backbuffer to the screen
    SwapChain->Present(0, 0);
}

