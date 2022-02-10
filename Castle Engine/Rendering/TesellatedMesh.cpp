#include "TesellatedMesh.hpp"
#include "Renderer3D.hpp"

#ifndef NEDITOR
	#include "../Editor/Editor.hpp"
#endif

TesellatedMesh::TesellatedMesh(ID3D11Device* _device, TerrainVertex* vertices, 
	unsigned int* indices) : device(_device)
{ 
	shader = new TesellationShader("Shaders/Tesellation.hlsl");
	m_layout = Renderer3D::CreateVertexInputLayout({ {"POSITION", DXGI_FORMAT_R32G32B32_FLOAT}, {"NORMAL",DXGI_FORMAT_R32G32B32_FLOAT} }, shader->VS_Buffer);

	DX_CREATE(D3D11_BUFFER_DESC, hullCBufferDesc);
	hullCBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	hullCBufferDesc.ByteWidth = sizeof(HS_CBuffer);
	hullCBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	hullCBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hullCBufferDesc.MiscFlags = 0;
	hullCBufferDesc.StructureByteStride = 0;
	
	DX_CREATE(D3D11_SUBRESOURCE_DATA, tessBufferData);
	tessBufferData.pSysMem = &HS_Buff;

	// Create the constant buffer pointer so we can access the hull shader constant buffer from within this class.
	DX_CHECK(
	device->CreateBuffer(&hullCBufferDesc, &tessBufferData, &HullConstBuffer), "tesellation buffer creation failed");
	
	DX_CREATE(D3D11_BUFFER_DESC, vertexBufferDesc);
		
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(TerrainVertex) * TERRAIN_VERTEX_COUNT;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
		
	DX_CREATE(D3D11_SUBRESOURCE_DATA, vertexBufferData);
	vertexBufferData.pSysMem = vertices;

	DX_CHECK(
	device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &vertexBuffer), "vertex buffer creation failed!");
		
	// create index buffer
	DX_CREATE(D3D11_BUFFER_DESC, indexDesc);
	indexDesc.Usage = D3D11_USAGE_DEFAULT;
	indexDesc.ByteWidth = sizeof(unsigned int) * TERRAIN_INDEX_COUNT;
	indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexDesc.CPUAccessFlags = 0;
	indexDesc.MiscFlags = 0;
		
	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = indices;
	device->CreateBuffer(&indexDesc, &initData, &indexBuffer);
}

void TesellatedMesh::Render(ID3D11DeviceContext* deviceContext, ID3D11Buffer* DSConstBuffer, const XMMATRIX& mvp, const glm::vec3& camPos)
{
	shader->Bind();

	DS_Buff.mvp = mvp;
	HS_Buff.cameraPos = camPos;

	deviceContext->UpdateSubresource(HullConstBuffer, 0, NULL, &HS_Buff, 0, 0);
	deviceContext->UpdateSubresource(DSConstBuffer  , 0, NULL, &DS_Buff, 0, 0);
	
	deviceContext->VSSetConstantBuffers(1, 1, &HullConstBuffer);
	deviceContext->HSSetConstantBuffers(1, 1, &HullConstBuffer);
	deviceContext->VSSetConstantBuffers(1, 1, &HullConstBuffer);
	deviceContext->PSSetConstantBuffers(1, 1, &HullConstBuffer);
	deviceContext->DSSetConstantBuffers(0, 1, &DSConstBuffer);

	const UINT stride = sizeof(TerrainVertex), offset = 0;
	// set index vertex buffers
	deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT , 0);
	deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

	// Set the type of primitive that should be rendered from this vertex buffer.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	
	deviceContext->DrawIndexed(TERRAIN_INDEX_COUNT, 0, 0);
	
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
		
		ImGui::DragFloat("noiseScale",  &DS_Buff.noiseScale , 0.01f);
		ImGui::DragFloat("bias", 		&DS_Buff.bias	    , 0.1f);
		ImGui::DragFloat("noiseHeight", &DS_Buff.noiseHeight, 0.1f);
	}
}
#endif

