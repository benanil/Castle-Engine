#include "TesellatedMesh.hpp"
#include "Renderer3D.hpp"

#ifndef NEDITOR
	#include "../Editor/Editor.hpp"
#endif

TesellatedMesh::TesellatedMesh(ID3D11Device* _device, const PointsAndIndices32& mesh) 
: device(_device), indexCount(mesh.indexCount)
{ 
	shader = new TesellationShader("Shaders/Tesellation.hlsl");

	D3D11_INPUT_ELEMENT_DESC inputLayoutInfo = 
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0};

	DirectxBackend::GetDevice()->CreateInputLayout(
		&inputLayoutInfo, 1, shader->VS_Buffer->GetBufferPointer(),
		shader->VS_Buffer->GetBufferSize(), &layout);

	DXCreateConstantBuffer(device, HullConstBuffer, &HS_Buff);
	
	DX_CREATE(D3D11_BUFFER_DESC, vertexBufferDesc);
		
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(glm::vec3) * mesh.vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		
	DX_CREATE(D3D11_SUBRESOURCE_DATA, vertexBufferData);
	vertexBufferData.pSysMem = mesh.vertices;

	DX_CHECK(
	device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &vertexBuffer), "vertex buffer creation failed!");
		
	// create index buffer
	DX_CREATE(D3D11_BUFFER_DESC, indexDesc);
	indexDesc.Usage = D3D11_USAGE_DEFAULT;
	indexDesc.ByteWidth = sizeof(uint32_t) * mesh.indexCount;
	indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		
	DX_CREATE(D3D11_SUBRESOURCE_DATA, initData);
	initData.pSysMem = mesh.indices;
	device->CreateBuffer(&indexDesc, &initData, &indexBuffer);
}

void TesellatedMesh::Render(ID3D11DeviceContext* deviceContext, const XMMATRIX& mvp, const glm::vec3& camPos)
{
	shader->Bind();

	HS_Buff.MVP = mvp;
	HS_Buff.cameraPos = camPos;

	deviceContext->UpdateSubresource(HullConstBuffer, 0, NULL, &HS_Buff, 0, 0);
	
	deviceContext->PSSetConstantBuffers(0, 1, &HullConstBuffer);
	deviceContext->HSSetConstantBuffers(0, 1, &HullConstBuffer);
	deviceContext->DSSetConstantBuffers(0, 1, &HullConstBuffer);

	const UINT stride = sizeof(glm::vec3), offset = 0;
	// set index vertex buffers
	deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT , 0);
	deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	deviceContext->IASetInputLayout(layout);

	// Set the type of primitive that should be rendered from this vertex buffer.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	
	deviceContext->DrawIndexed(indexCount, 0, 0);
	
	// reset
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->HSSetShader(0, 0, 0);
	deviceContext->DSSetShader(0, 0, 0);
}

#ifndef NEDITOR
void TesellatedMesh::OnEditor()
{
	if (ImGui::CollapsingHeader("Tesselation"))
	{
		ImGui::DragFloat("TessellationAmount", &HS_Buff.tessellationAmount, 0.1f);
		
		ImGui::DragFloat("noiseScale",  &HS_Buff.noiseScale , 0.01f);
		ImGui::DragFloat("bias"      ,  &HS_Buff.bias	    , 0.1f);
		ImGui::DragFloat("SunAngle"  ,  &HS_Buff.sunAngle, 0.1f);
		ImGui::DragFloat("noiseHeight", &HS_Buff.noiseHeight, 0.1f);
	}
}
#endif

