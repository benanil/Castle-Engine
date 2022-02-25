#include "DirectxBackend.hpp"
#include <D3DX10.h>
#include "SDL.h"
#include "Rendering.hpp"
#include "Engine.hpp"

namespace DirectxBackend
{
	IDXGISwapChain* SwapChain;
	ID3D11Device* Device;
	ID3D11DeviceContext* DeviceContext;

	ID3D11RenderTargetView* renderTargetView;
	ID3D11DepthStencilView* depthStencilView;
	ID3D11Texture2D* depthStencilBuffer;
	D3D11_VIEWPORT ViewPort;
	UINT MSAASamples;
	D3D11_TEXTURE2D_DESC depthDesc;

    void CleanupRenderTarget();
}

// getters
ID3D11Device* DirectxBackend::GetDevice() { return Device; }
ID3D11DeviceContext* DirectxBackend::GetDeviceContext() { return DeviceContext; }
unsigned int DirectxBackend::GetMSAASamples() { return MSAASamples; }

WindowSize DirectxBackend::GetWindowSize() { return{ ViewPort.Width, ViewPort.Height }; }
D3D11_VIEWPORT DirectxBackend::GetViewPort() { return ViewPort; }

void DirectxBackend::SetBackBufferAsRenderTarget() {
    DeviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
}

void DirectxBackend::ClearBackBuffer() {
    float bgColor[4] = { .4, .4, .7, 1.0f };
    DeviceContext->ClearRenderTargetView(renderTargetView, bgColor);
    DeviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void DirectxBackend::Present(bool Vsync)
{
	SwapChain->Present(Vsync, 0);
}

void DirectxBackend::CreateRenderTarget(SDL_Window* window)
{
    CleanupRenderTarget();
    SwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

    memset(&depthDesc, 0, sizeof(D3D11_TEXTURE2D_DESC));
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = std::max(MSAASamples, 1u);
    depthDesc.SampleDesc.Quality = MSAASamples;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    SDL_GetWindowSize(window, reinterpret_cast<int*>(&depthDesc.Width), reinterpret_cast<int*>(&depthDesc.Height));

    Device->CreateTexture2D(&depthDesc, NULL, &depthStencilBuffer);
    Device->CreateDepthStencilView(depthStencilBuffer, NULL, &depthStencilView);

    //Create our BackBuffer
    ID3D11Texture2D* BackBuffer;
    SwapChain->GetBuffer(0, IID_PPV_ARGS(&BackBuffer));

    DX_CREATE(D3D11_RENDER_TARGET_VIEW_DESC, rtvDesc);

    D3D11_TEXTURE2D_DESC backDesc;
    BackBuffer->GetDesc(&backDesc);

    rtvDesc.Format = backDesc.Format;
#ifndef NEDITOR
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
#else
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
#endif

    //Create our Render Target
    Device->CreateRenderTargetView(BackBuffer, &rtvDesc, &renderTargetView);
    BackBuffer->Release();

    //Set our Render Target
    DeviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

    //Update the Viewport
    memset(&ViewPort, 0, sizeof(D3D11_VIEWPORT));
    ViewPort.Width = depthDesc.Width;
    ViewPort.Height = depthDesc.Height;
    ViewPort.MaxDepth = 1.0f;

    DeviceContext->RSSetViewports(1, &ViewPort);

}

void DirectxBackend::InitializeDirect3d11App(SDL_Window* window)
{
    UINT creationFlags =
#ifndef NEDITOR
        D3D11_CREATE_DEVICE_DEBUG; // todo make 0 after development
#else
        D3D11_CREATE_DEVICE_DEBUG;
#endif

    DX_CHECK(
    D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, creationFlags, NULL, NULL, D3D11_SDK_VERSION, &Device, NULL, &DeviceContext),
    "Device Creation Failed!")

#ifndef NEDITOR
    MSAASamples = 0;
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
    HRESULT hr = Device->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDXGIDevice);

    IDXGIAdapter* pDXGIAdapter = nullptr;
    hr = pDXGIDevice->GetAdapter(&pDXGIAdapter);

    IDXGIFactory* pIDXGIFactory = nullptr;
    pDXGIAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&pIDXGIFactory);

    //Describe our Buffer
    DX_CREATE(DXGI_MODE_DESC, bufferDesc);
    bufferDesc.RefreshRate.Numerator = 60;
    bufferDesc.RefreshRate.Denominator = 1;
    bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    //Describe our SwapChain
    DX_CREATE(DXGI_SWAP_CHAIN_DESC, swapChainDesc);

    swapChainDesc.BufferDesc = bufferDesc;
    swapChainDesc.SampleDesc.Count = std::max(MSAASamples, 1u);
    swapChainDesc.SampleDesc.Quality = MSAASamples;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 1;
    swapChainDesc.OutputWindow = Engine::GetHWND();
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    hr = pIDXGIFactory->CreateSwapChain(Device, &swapChainDesc, &SwapChain);

    //Create the Viewport
    memset(&ViewPort, 0, sizeof(D3D11_VIEWPORT));

    ViewPort.Width = 1000;
    ViewPort.Height = 800;
    ViewPort.MaxDepth = 1.0f;

    //Set the Viewport
    DeviceContext->RSSetViewports(1, &ViewPort);
    std::cout << "initialized" << std::endl;

    CreateRenderTarget(window);
}

void DirectxBackend::CleanupRenderTarget()
{
    DX_RELEASE(renderTargetView)
    DX_RELEASE(depthStencilBuffer)
    DX_RELEASE(depthStencilView)
}

void DirectxBackend::Destroy()
{
    SwapChain->Release();
    Device->Release();
    DeviceContext->Release();
    renderTargetView->Release();
    depthStencilView->Release();
    depthStencilBuffer->Release();
}