#pragma once
#include "RenderTexture.hpp"
#include <xnamath.h>
#include "../FreeCamera.hpp"

#ifndef CB_GlobalDefined
# define CB_GlobalDefined
struct cbGlobal
{
	float sunAngle;
	glm::vec3 ambientColor;  // 16
	glm::vec3 sunColor;
	float additionalData;      // 32
	glm::vec3 viewPos;
	float ambientStength; // 48
};
#endif
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
	FreeCamera* GetCamera();
	void AddMeshRenderer(MeshRenderer* meshRenderer);
	void DrawScene();
	void RenderMeshes(D3D11_VIEWPORT* viewPort );
	void InvalidateRenderTexture(int width, int height);
	// do not send matrix transposed!
	void SetMVP(const XMMATRIX& model, const XMMATRIX& viewProjection);
	void SetModelMatrix(const XMMATRIX& matrix);
	cbGlobal& GetGlobalCbuffer();
	//  <summary> before this ready-set your rendertexture and shaders </summary>
	void RenderToQuad();

#ifndef NEDITOR
	void OnEditor();
#endif
	void Dispose();
}
