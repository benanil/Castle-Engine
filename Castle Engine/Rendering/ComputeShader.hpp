#pragma once

#include <D3DX11.h>
#include <glm/glm.hpp>
#include <vector>
#ifdef _MSC_VER
#pragma comment(lib, "d3dcompiler") // Automatically link with d3dcompiler.lib as we are using D3DCompile() below.
#endif

namespace CS {
	struct BufferMappingResult {
		void* data; ID3D11Buffer* mOutputDebugBuffer;
	};

	struct TextureCreateResult {
		unsigned int textureIndex, UAV_Index, width, height;
	};

	struct TextureMappingResult {
		void* data; ID3D11Texture2D* mOutputDebugBuffer;
	};

	struct CreationResult {
		int UAV_Index; unsigned int stride, numElements;
	};
}

class ComputeShader
{
public:
	ID3D11ComputeShader* CS = nullptr;
	ID3D10Blob* CS_Buffer = nullptr;
	std::vector<ID3D11Texture2D*> Textures;
	std::vector<ID3D11Buffer*> GPUBuffers;
	std::vector<ID3D11ShaderResourceView*> resourceWiews;
	std::vector<ID3D11UnorderedAccessView*> UAVs;

public:
	ComputeShader() {};
	ComputeShader(const char* path, const char* CSName, unsigned short groupCountX, unsigned short groupCountY);
	
	/// <returns> registered index </returns>
	CS::CreationResult RWCreateUAVBuffer(unsigned int stride, unsigned int numElements, void* data);
	CS::BufferMappingResult RWMapUAVBuffer(const CS::CreationResult& creationResult, D3D11_MAP mappingMode);
	void RWUnmapUAVBuffer(ID3D11Buffer* mOutputBuffer);

	CS::TextureCreateResult RWCreateUAVTexture(unsigned int width, unsigned int height);
	CS::TextureMappingResult RWMapUAVTexture(const CS::TextureCreateResult&, D3D11_MAP mappingMode);
	void RWUnmapUAVTexture(ID3D11Texture2D* mOutputBuffer);

	/// <returns> registered index </returns>
	CS::CreationResult CreateStructuredBuffer(unsigned int stride, unsigned int numberOfElements, void* initData);
	CS::BufferMappingResult MapStructuredBuffer(int index, D3D11_MAP mappingMode);
	void UnmapStructuredBuffer(ID3D11Buffer* mOutputBuffer);

	void Dispatch();
	
	void Dispose();
	
	~ComputeShader();
	
private:
	
	void ReleaseGPUBuffers();
	void ReleaseTextureBuffers();
	void ReleaseUAVs();

	unsigned short groupCountX, groupCountY;
	ID3D11Device* device;
	ID3D11DeviceContext* d3d11DevCon;
};