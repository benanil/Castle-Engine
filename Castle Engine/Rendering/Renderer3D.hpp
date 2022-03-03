#pragma once
#include "RenderTexture.hpp"
#include <xnamath.h>

struct InputLayoutCreateInfo
{
	const char* name; DXGI_FORMAT format;
};

class FreeCamera;
class MeshRenderer;
	
namespace Renderer3D
{
	void Initialize(FreeCamera* freeCamera);
	
	RenderTexture* GetPostRenderTexture();
	void AddMeshRenderer(MeshRenderer* meshRenderer);
	// do not send matrix transposed!
	void SetModelMatrix(const XMMATRIX& matrix);
	void DrawScene();
	void RenderMeshes();
	void InvalidateRenderTexture(int width, int height);

	//  <summary> before this ready-set your rendertexture and shaders </summary>
	void RenderToQuad();

#ifndef NEDITOR
	void OnEditor();
#endif
	void Dispose();
}
