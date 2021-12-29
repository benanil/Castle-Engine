#pragma once

#include "../Helper.hpp"
#include "../Engine.hpp"

struct cbGlobal
{
	float sunAngle;
	glm::vec3 ambientColor;  // 16
	glm::vec3 sunColor;      
	float fogAmount;      // 32
	glm::vec3 viewPos;
	float ambientStength; // 48
};

struct cbPerObject
{
	XMMATRIX  MVP;
	XMMATRIX  Model;
};

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 texCoord;
	glm::vec3 tangent;

	static DXInputLayout* GetLayout(DXBlob* VS_Buffer)
	{
		DXInputLayout* vertLayout;

		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 , D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL"  , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT   , 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT" , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		auto device = Engine::GetDevice();
		device->CreateInputLayout(layout, 4, VS_Buffer->GetBufferPointer(),
			VS_Buffer->GetBufferSize(), &vertLayout);
		return vertLayout;
	}
};

struct SkyboxVertex
{
	glm::vec3 pos;
	
	static DXInputLayout* GetLayout(DXBlob* VS_Buffer)
	{
		DXInputLayout* vertLayout;
		
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 , D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		
		auto device = Engine::GetDevice();
		device->CreateInputLayout(layout, 1, VS_Buffer->GetBufferPointer(),
			VS_Buffer->GetBufferSize(), &vertLayout);
		return vertLayout;
	}
};

template<typename TVertex>
class Sphere
{
public:
	DXBuffer* indexBuffer;
	DXBuffer* vertexBuffer;

	uint16_t NumVertices;
	uint16_t NumFaces;
	
	Sphere() {};
	
	Sphere(uint8_t LatLines, uint8_t LongLines)
	{
		NumVertices = ((LatLines - 2) * LongLines) + 2;
		NumFaces = ((LatLines - 3) * (LongLines) * 2) + (LongLines * 2);

		float sphereYaw = 0.0f;
		float spherePitch = 0.0f;

		// calculate vertices
		std::vector<TVertex> vertices(NumVertices);

		XMVECTOR currVertPos = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

		vertices[0].pos.x = 0.0f;
		vertices[0].pos.y = 0.0f;
		vertices[0].pos.z = 1.0f;

		for (uint16_t i = 0; i < LatLines - 2; ++i)
		{
			spherePitch = (i + 1) * (DX_PI / (LatLines - 1));
			auto Rotationx = XMMatrixRotationX(spherePitch);
			for (uint16_t j = 0; j < LongLines; ++j)
			{
				sphereYaw = j * (DX_TWO_PI / (LongLines));
				auto Rotationy = XMMatrixRotationZ(sphereYaw);
				currVertPos = XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), (Rotationx * Rotationy));
				currVertPos = XMVector3Normalize(currVertPos);
				vertices[i * LongLines + j + 1].pos.y = XMVectorGetY(currVertPos);
				vertices[i * LongLines + j + 1].pos.x = XMVectorGetX(currVertPos);
				vertices[i * LongLines + j + 1].pos.z = XMVectorGetZ(currVertPos);
			}
		}

		vertices[NumVertices - 1].pos.x = 0.0f;
		vertices[NumVertices - 1].pos.y = 0.0f;
		vertices[NumVertices - 1].pos.z = -1.0f;

		DX_CREATE(D3D11_BUFFER_DESC, vertexBufferDesc);
		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(TVertex) * NumVertices;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;

		auto d3d11Device = Engine::GetDevice();

		DX_CREATE(D3D11_SUBRESOURCE_DATA, vertexBufferData);
		vertexBufferData.pSysMem = &vertices[0];
		HRESULT hr = d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &vertexBuffer);

		// calculate indices
		std::vector<uint16_t> indices(NumFaces * 3);

		int k = 0;
		for (uint16_t l = 0; l < LongLines - 1; ++l)
		{
			indices[k] = 0;
			indices[k + 1] = l + 1;
			indices[k + 2] = l + 2;
			k += 3;
		}

		indices[k] = 0;
		indices[k + 1] = LongLines;
		indices[k + 2] = 1;
		k += 3;

		for (uint16_t i = 0; i < LatLines - 3; ++i)
		{
			for (uint16_t j = 0; j < LongLines - 1; ++j)
			{
				indices[k] = i * LongLines + j + 1;
				indices[k + 1] = i * LongLines + j + 2;
				indices[k + 2] = (i + 1) * LongLines + j + 1;

				indices[k + 3] = (i + 1) * LongLines + j + 1;
				indices[k + 4] = i * LongLines + j + 2;
				indices[k + 5] = (i + 1) * LongLines + j + 2;

				k += 6; // next quad
			}

			indices[k] = (i * LongLines) + LongLines;
			indices[k + 1] = (i * LongLines) + 1;
			indices[k + 2] = ((i + 1) * LongLines) + LongLines;

			indices[k + 3] = ((i + 1) * LongLines) + LongLines;
			indices[k + 4] = (i * LongLines) + 1;
			indices[k + 5] = ((i + 1) * LongLines) + 1;

			k += 6;
		}

		for (uint16_t l = 0; l < LongLines - 1; ++l)
		{
			indices[k] = NumVertices - 1;
			indices[k + 1] = (NumVertices - 1) - (l + 1);
			indices[k + 2] = (NumVertices - 1) - (l + 2);
			k += 3;
		}
		
		indices[k] = NumVertices - 1;
		indices[k + 1] = (NumVertices - 1) - LongLines;
		indices[k + 2] = NumVertices - 2;

		DX_CREATE(D3D11_BUFFER_DESC, indexBufferDesc);
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(uint16_t) * NumFaces * 3;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA iinitData;
		iinitData.pSysMem = &indices[0];
		d3d11Device->CreateBuffer(&indexBufferDesc, &iinitData, &indexBuffer);
	}

	void Draw()
	{
		auto context = Engine::GetDeviceContext();
		context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);

		UINT stride = sizeof(TVertex);
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
		context->DrawIndexed(NumFaces * 3, 0, 0 );
	}

	~Sphere()
	{
		Dispose();
	}

	void Dispose()
	{
		DX_RELEASE(indexBuffer)
		DX_RELEASE(vertexBuffer)
	}
};

typedef Sphere<SkyboxVertex> SphereSky;