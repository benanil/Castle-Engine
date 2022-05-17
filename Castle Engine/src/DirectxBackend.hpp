#pragma once
#include <d3dx11.h>

struct SDL_Window;

struct WindowSize { float Width, Height; };

namespace DirectxBackend
{
	void CreateRenderTarget(SDL_Window* window);
	void InitializeDirect3d11App(SDL_Window* window);
	void Destroy();

	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetDeviceContext();
	unsigned int GetMSAASamples();

	void SetBackBufferAsRenderTarget();
	void ClearBackBuffer();

	void Present(bool Vsync);
	WindowSize GetWindowSize();
	D3D11_VIEWPORT GetViewPort();
}

