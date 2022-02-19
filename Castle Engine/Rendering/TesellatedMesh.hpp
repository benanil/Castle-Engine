#pragma once
#include "TesellationShader.hpp"
#include "Mesh.hpp"

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
	TesellatedMesh(ID3D11Device* _device, const PointsAndIndices32& mesh);
	void Render(ID3D11DeviceContext* deviceContext, const XMMATRIX& mvp, const glm::vec3& position);
	void OnEditor();
private:
	TesellationShader* shader;
	ID3D11InputLayout* layout;
	ID3D11Device* device;
	ID3D11Buffer* HullConstBuffer;
	DXBuffer* vertexBuffer, *indexBuffer;
	uint32_t indexCount;
};

