#pragma once
#include "TesellationShader.hpp"
#include "Terrain.hpp"

class TesellatedMesh
{
public:
	__declspec(align(128)) struct HS_CBuffer 
	{ 
		XMMATRIX MVP;
		float noiseScale = 0.2, bias = 0, noiseHeight = 30;
		float tessellationAmount = 0.5;
		glm::vec3 cameraPos;
		float sunAngle = 0.2f;
	} HS_Buff;
	
	TesellatedMesh(){};
	TesellatedMesh(ID3D11Device* _device, TerrainVertex* vertices, uint32_t* indices);
	void Render(ID3D11DeviceContext* deviceContext, const XMMATRIX& mvp, const glm::vec3& position);
	void OnEditor();
private:
	TesellationShader* shader;
	ID3D11InputLayout* m_layout;
	ID3D11Device* device;
	ID3D11Buffer* HullConstBuffer;
	DXBuffer* vertexBuffer, *indexBuffer;
};

