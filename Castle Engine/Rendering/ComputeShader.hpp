#pragma once

#include <D3DX11.h>
#include <glm/glm.hpp>
#include <vector>
#include "Texture.hpp"
#ifdef _MSC_VER
#pragma comment(lib, "d3dcompiler") // Automatically link with d3dcompiler.lib as we are using D3DCompile() below.
#endif

namespace CS {
	struct BufferMappingResult {
		void* data; ID3D11Buffer* mOutputDebugBuffer;
	};

	struct TextureCreateResult {
		ID3D11ShaderResourceView* resourceViewIndex; unsigned int UAV_Index, width, height;
	};

	struct TextureMappingResult {
		void* data; ID3D11Texture2D* mOutputDebugBuffer;
	};

	struct RWBufferHandle {
		unsigned int UAV_Index, stride, numElements, GPUBufferIndex, resourceViewIndex;
	};
	struct StructuredBufferHandle {
		unsigned int GPUBufferIndex; D3D11_MAP mapMode; unsigned int resourceViewIndex;
	};
}

class ComputeShader
{
private:
	ID3D11ComputeShader* CS = nullptr;
	ID3D10Blob* CS_Buffer = nullptr;
	std::vector<ID3D11Texture2D*> Textures;
	std::vector<ID3D11Buffer*> GPUBuffers;
	std::vector<ID3D11ShaderResourceView*> resourceViews;
	std::vector<ID3D11UnorderedAccessView*> UAVs;
	unsigned int groupCountX, groupCountY;
	ID3D11Device* device;
	ID3D11DeviceContext* d3d11DevCon;
public:
	ComputeShader() {};
	ComputeShader(const char* path, const char* CSName, unsigned int groupCountX, unsigned int groupCountY);
	
	CS::RWBufferHandle RWCreateUAVBuffer(unsigned int stride, unsigned int numElements, void* data);
	CS::BufferMappingResult RWMapUAVBuffer(const CS::RWBufferHandle& creationResult, D3D11_MAP mappingMode);

	/// <returns> registered resourceWiew index </returns>
	CS::TextureCreateResult RWAddAsUAVTexture(const Texture& texture);
	CS::TextureCreateResult RWCreateUAVTexture(unsigned int width, unsigned int height, void* pixels = nullptr, D3D11_MAP mappingMode = (D3D11_MAP)0);
	// <summary>for accessing texture from cpu </summary>
	CS::TextureMappingResult RWMapUAVTexture(const CS::TextureCreateResult&, D3D11_MAP mappingMode);
	void RWSetUAVTexture(unsigned int index, ID3D11ShaderResourceView* resourceView);

	CS::StructuredBufferHandle CreateStructuredBuffer(unsigned int stride, unsigned int numberOfElements, void* initData, D3D11_MAP mappingMode = (D3D11_MAP)0);
	CS::BufferMappingResult MapStructuredBuffer(CS::StructuredBufferHandle SBCreateResult);

	/// <summary> added texture index </summary>
	unsigned int AddTexture(ID3D11ShaderResourceView* resource);
	void SetTexture(unsigned int index, ID3D11ShaderResourceView* resource);
	ID3D11ShaderResourceView* GetTexture(unsigned int index);

	void UnmapBuffer(ID3D11Buffer* mOutputBuffer);
	void UnmapTexture(ID3D11Texture2D* mOutputBuffer);

	void SetThreadGroups(unsigned int x, unsigned int y) { groupCountX = x; groupCountY = y; };

	void Dispatch();
	void Dispatch(unsigned int x, unsigned int y) { SetThreadGroups(x, y); Dispatch();  };
	void Dispose();

	void ReleaseAllBuffers();
	void ReleaseGPUBuffer(const CS::StructuredBufferHandle& handle);
	void RWReleaseUAV(const CS::RWBufferHandle& handle);
	
	~ComputeShader();
	
private:
	__forceinline static D3D11_CPU_ACCESS_FLAG MapModeToCPUAccesFlag(D3D11_MAP mappingMode) {
		switch (mappingMode)
		{
		case 0: return (D3D11_CPU_ACCESS_FLAG)0;
		case D3D11_MAP_READ:       return D3D11_CPU_ACCESS_READ;
		case D3D11_MAP_WRITE:      return D3D11_CPU_ACCESS_WRITE;
		default: return (D3D11_CPU_ACCESS_FLAG)0;
		}
	}
	ID3D11UnorderedAccessView* CreateUAVFromResourceView(ID3D11ShaderResourceView* resourceView);
	void ReleaseGPUBuffers();
	void ReleaseTextureBuffers();
	void ReleaseUAVs();
};