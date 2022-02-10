#pragma once
#include "TesellationShader.hpp"
#include "Terrain.hpp"

class TesellatedMesh
{
public:
	__declspec(align(16)) struct HS_CBuffer 
	{ // hull segment
		float tessellationAmount = 0.5;
		glm::vec3 cameraPos;
	} HS_Buff;
	
	__declspec(align(128)) struct DS_CBuffer { // domain segment
		XMMATRIX mvp;
		float noiseScale = 0.2, bias = 0, noiseHeight = 30;
	} DS_Buff;
	TesellatedMesh(){};
	TesellatedMesh(ID3D11Device* _device, TerrainVertex* vertices, unsigned int* indices);
	void Render(ID3D11DeviceContext* deviceContext, ID3D11Buffer* cbPerObj, const XMMATRIX& mvp, const glm::vec3& position);
	void OnEditor();
private:
	TesellationShader* shader;
	ID3D11InputLayout* m_layout;
	ID3D11Device* device;
	ID3D11Buffer* HullConstBuffer;
	DXBuffer* vertexBuffer, *indexBuffer;
};

