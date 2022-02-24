
#include "ComputeShader.hpp"

#include <fstream>
#include <string>
#include <spdlog/spdlog.h>
#include <vector>
#include <filesystem>
#include <cassert>
#include <d3dcompiler.h>
#include <cstdint>
#include "../Engine.hpp"
#ifdef _MSC_VER
	#pragma comment(lib, "d3dcompiler") // Automatically link with d3dcompiler.lib as we are using D3DCompile() below.
#endif

#include "../Rendering.hpp"
#include "../DirectxBackend.hpp"
#include "../Helper.hpp"

using namespace CS;

ComputeShader::ComputeShader(
	const char* path, const char* CSName, uint32_t _width, uint32_t _height)
{
	groupCountX = _width; groupCountY = _height;
	device = DirectxBackend::GetDevice();
	d3d11DevCon = DirectxBackend::GetDeviceContext();

	LPCSTR profile = (device->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0) ? "cs_5_0" : "cs_4_0";
	
	std::string computeShader = ReadAllText(std::string(path));
	
	DXBlob* computeErrorBlob;
	
	if (FAILED(D3DCompile(computeShader.c_str(), computeShader.size(), nullptr,
		nullptr, nullptr, "CS", profile, 0, 0, &CS_Buffer, &computeErrorBlob)))
	{
		std::cout << "Compute Shader Compiling Error:\n" << ((char*)computeErrorBlob->GetBufferPointer()) << std::endl;
		DX_CHECK(-1, "Compute Shader Compiling Error")
	}
	
	HRESULT hr = device->CreateComputeShader(CS_Buffer->GetBufferPointer(), CS_Buffer->GetBufferSize(), NULL, &CS);
	
	if (hr != S_OK) {
		std::cout << "Compute Shader compiling error!\n hresult: " << hr << std::endl; DX_CHECK(-1, "Compute Shader Compiling Error ")
	}
	
	DX_RELEASE(computeErrorBlob);
}

void ComputeShader::Dispatch()
{
	ID3D11UnorderedAccessView* pUAViewNULL[1] = { NULL };
	ID3D11ShaderResourceView* pSRVNULL[2] = { NULL, NULL };
	UINT count = 1;
	d3d11DevCon->VSSetShader(nullptr, 0, 0);
	d3d11DevCon->PSSetShader(nullptr, 0, 0);
	d3d11DevCon->CSSetShader(CS, 0, 0);
	d3d11DevCon->CSSetUnorderedAccessViews(0, UAVs.size(), UAVs.data(), &count);
	d3d11DevCon->CSSetShaderResources(0, resourceViews.size(), resourceViews.data());
	d3d11DevCon->Dispatch(groupCountX, groupCountY, 1);

	d3d11DevCon->CSSetShader(nullptr, 0, 0);
	d3d11DevCon->CSSetUnorderedAccessViews(0, 1, pUAViewNULL, NULL);
	d3d11DevCon->CSSetShaderResources(0, 2, pSRVNULL);
}

/// <returns> registered index </returns>
RWBufferHandle ComputeShader::RWCreateUAVBuffer(unsigned int stride, unsigned int numElements, void* data)
{
	DX_CREATE(D3D11_BUFFER_DESC, RWBufferDesc);
	RWBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	RWBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	RWBufferDesc.StructureByteStride = stride;
	RWBufferDesc.ByteWidth = stride * numElements;
	RWBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	RWBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

	DX_CREATE(D3D11_SUBRESOURCE_DATA, vinitData);
	vinitData.pSysMem = data;
	D3D11_SUBRESOURCE_DATA* p_vinitData = data ? &vinitData : nullptr;
	ID3D11Buffer* GpuBuffer;
	HRESULT hr = device->CreateBuffer(&RWBufferDesc, p_vinitData, &GpuBuffer);

	GPUBuffers.push_back(GpuBuffer);

	if (FAILED(hr)) {
		std::cout << "RW buffer creation failed! with message: " << hr << std::endl;
	}

	DX_CREATE(D3D11_UNORDERED_ACCESS_VIEW_DESC, uavDesc) ;
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_COUNTER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = numElements;

	ID3D11UnorderedAccessView* UAV;
	hr = device->CreateUnorderedAccessView(GpuBuffer, &uavDesc, &UAV);
	UAVs.push_back(UAV);
	if (FAILED(hr)) {
		std::cout << "RW UAV creation failed! with message:" << hr << std::endl;
	}

	RWBufferHandle result{ UAVs.size() - 1, stride, numElements, UAVs.size() -1, resourceViews.size() - 1};
	
	return result;
}

BufferMappingResult ComputeShader::RWMapUAVBuffer(const RWBufferHandle& creationResult, D3D11_MAP mappingMode)
{
	auto UAV = UAVs[creationResult.UAV_Index];
	DX_CREATE(D3D11_BUFFER_DESC, outputDesc);
	outputDesc.Usage = D3D11_USAGE_STAGING;
	outputDesc.BindFlags = 0;
	outputDesc.ByteWidth = creationResult.stride * creationResult.numElements;
	outputDesc.CPUAccessFlags = MapModeToCPUAccesFlag(mappingMode);
	outputDesc.StructureByteStride = creationResult.stride;
	outputDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	ID3D11Buffer* OutputBuffer;
	DX_CHECK(device->CreateBuffer(&outputDesc, 0, &OutputBuffer), "RW UAV buffer mapping staging buffer creation failed!");
	ID3D11Resource* resource;
	UAV->GetResource(&resource);

	d3d11DevCon->CopyResource(OutputBuffer, resource);
	if (OutputBuffer == nullptr) {
		std::cout << "mOutputDebugBuffer is null" << std::endl;
	}

	DX_CREATE(D3D11_MAPPED_SUBRESOURCE, mappedData);
	CS::BufferMappingResult result{};
	if (!FAILED(d3d11DevCon->Map(OutputBuffer, 0, mappingMode, 0, &mappedData)))
	{
		result.data = mappedData.pData;
	} else assert(1, "RW UAV mapping failed!");
	result.OutputBuffer = OutputBuffer;
	return result;
}

TextureCreateResult ComputeShader::RWCreateUAVTexture(uint32_t width, uint32_t height, void* pixels, D3D11_MAP mappingMode)
{
	DX_CREATE(D3D11_TEXTURE2D_DESC, textureDesc);
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	textureDesc.CPUAccessFlags = MapModeToCPUAccesFlag(mappingMode);
	textureDesc.MiscFlags = 0;
	ID3D11Texture2D* texture = 0;
	DX_CREATE(D3D11_SUBRESOURCE_DATA, vinitData);
	vinitData.pSysMem = pixels;
	D3D11_SUBRESOURCE_DATA* p_vinitData = pixels ? &vinitData : nullptr;

	DX_CHECK(device->CreateTexture2D(&textureDesc, p_vinitData, &texture), "uav creation failed!") ;

	DX_CREATE(D3D11_SHADER_RESOURCE_VIEW_DESC, srvDesc);
	srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	ID3D11ShaderResourceView* mOutputTexSRV;
	DX_CHECK(device->CreateShaderResourceView(texture, &srvDesc, &mOutputTexSRV), "uav creation failed!") ;
	
	DX_CREATE(D3D11_UNORDERED_ACCESS_VIEW_DESC, uavDesc);
	uavDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	ID3D11UnorderedAccessView* mOutputTexUAV;
	DX_CHECK(device->CreateUnorderedAccessView(texture, &uavDesc, &mOutputTexUAV), "uav creation failed!");

	TextureCreateResult result{ mOutputTexSRV, UAVs.size() , width, height};
	Textures.push_back(texture);
	UAVs.push_back(mOutputTexUAV);
	
	return result; 
}

TextureMappingResult ComputeShader::RWMapUAVTexture(const TextureCreateResult& texResult, D3D11_MAP mappingMode)
{
	ID3D11UnorderedAccessView* UAV = UAVs[texResult.UAV_Index];
	DX_CREATE(D3D11_TEXTURE2D_DESC, outputDesc);
	outputDesc.Width = texResult.width;
	outputDesc.Height = texResult.height;
	outputDesc.MipLevels = 1;
	outputDesc.ArraySize = 1;
	outputDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	outputDesc.SampleDesc.Count = 1;
	outputDesc.SampleDesc.Quality = 0;
	outputDesc.Usage = D3D11_USAGE_DEFAULT;
	outputDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	outputDesc.CPUAccessFlags = MapModeToCPUAccesFlag(mappingMode);
	outputDesc.MiscFlags = 0;
	ID3D11Texture2D* OutputBuffer;
	DX_CHECK(device->CreateTexture2D(&outputDesc, 0, &OutputBuffer), "RW UAV texture creation failed!");
	ID3D11Resource* resource;
	UAV->GetResource(&resource);

	d3d11DevCon->CopyResource(OutputBuffer, resource);
	D3D11_MAPPED_SUBRESOURCE mappedData;
	if (OutputBuffer == nullptr) {
		std::cout << "mOutputDebugBuffer is null" << std::endl;
	}
	CS::TextureMappingResult result{};
	if (!FAILED(d3d11DevCon->Map(OutputBuffer, 0, mappingMode, 0, &mappedData))) {
		result.data = mappedData.pData;
	}
	result.OutputBuffer = OutputBuffer;
	return result;
}

TextureCreateResult ComputeShader::RWAddAsUAVTexture(const RenderTexture* texture)
{
	ID3D11UnorderedAccessView* mOutputTexUAV = CreateUAVFromResourceView(texture->textureView);
	TextureCreateResult result{ texture->textureView, UAVs.size() , texture->width, texture->height };
	UAVs.push_back(mOutputTexUAV);
	return result;
}

TextureCreateResult ComputeShader::RWAddAsUAVTexture(const Texture& texture)
{
	ID3D11UnorderedAccessView* mOutputTexUAV = CreateUAVFromResourceView(texture.resourceView);
	TextureCreateResult result{ texture.resourceView, UAVs.size() , texture.width, texture.height };
	UAVs.push_back(mOutputTexUAV);
	return result;
}
 
void ComputeShader::RWSetUAVTexture(unsigned int uavIndex, const Texture& texture)
{
	ID3D11UnorderedAccessView* mOutputTexUAV = CreateUAVFromResourceView(texture.resourceView);
	DX_RELEASE(UAVs[uavIndex]);
	UAVs[uavIndex] = mOutputTexUAV;
}

void ComputeShader::RWSetUAVTexture(unsigned int uavIndex, const RenderTexture* texture)
{
	ID3D11UnorderedAccessView* mOutputTexUAV = CreateUAVFromResourceView(texture->textureView);
	DX_RELEASE(UAVs[uavIndex]);
	UAVs[uavIndex] = mOutputTexUAV;
}

void ComputeShader::RWSetUAVTexture(unsigned int index, ID3D11ShaderResourceView* resourceView)
{
	ID3D11UnorderedAccessView* createdUAV = CreateUAVFromResourceView(resourceView);
	if (Textures.size() > index) {
		DX_RELEASE(UAVs[index]);
		UAVs[index] = createdUAV;
	}
	else UAVs.push_back(createdUAV);
}

ID3D11UnorderedAccessView* ComputeShader::CreateUAVFromResourceView(ID3D11ShaderResourceView* resourceView)
{
	ID3D11UnorderedAccessView* result;
	DX_CREATE(D3D11_UNORDERED_ACCESS_VIEW_DESC, uavDesc);
	uavDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	ID3D11Resource* textureBuffer;
	resourceView->GetResource(&textureBuffer);
	DX_CHECK(device->CreateUnorderedAccessView(textureBuffer, &uavDesc, &result), "");
	return result;
}

/// <returns> registered index </returns>
StructuredBufferHandle ComputeShader::CreateStructuredBuffer(
	unsigned int stride, unsigned int numElements, void* data, D3D11_MAP mappingMode)
{
	DX_CREATE(D3D11_BUFFER_DESC, inputDesc);
	inputDesc.Usage = D3D11_USAGE_DEFAULT;
	inputDesc.ByteWidth = numElements * stride;
	inputDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	inputDesc.CPUAccessFlags = MapModeToCPUAccesFlag(mappingMode);
	inputDesc.StructureByteStride = stride;
	inputDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	DX_CREATE(D3D11_SUBRESOURCE_DATA, vinitData);
	vinitData.pSysMem = data;
	D3D11_SUBRESOURCE_DATA* p_vinitData = data ? &vinitData : nullptr;

	ID3D11Buffer* buffer;
	DX_CHECK(device->CreateBuffer(&inputDesc, p_vinitData, &buffer), "structured buffer creation failed");
	GPUBuffers.push_back(buffer);

	DX_CREATE(D3D11_SHADER_RESOURCE_VIEW_DESC, srvDesc);
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	srvDesc.BufferEx.NumElements = numElements;

	ID3D11ShaderResourceView* srv;
	DX_CHECK(device->CreateShaderResourceView(buffer, &srvDesc, &srv), "Structured Buffer SRV creation failed!");

	StructuredBufferHandle result{ GPUBuffers.size() - 1 , mappingMode, resourceViews.size() };
	resourceViews.push_back(srv);

	return result;
}

BufferMappingResult ComputeShader::MapStructuredBuffer(StructuredBufferHandle SBCreateResult)
{
	D3D11_MAPPED_SUBRESOURCE mappedData;

	CS::BufferMappingResult result{};
	if (!FAILED(d3d11DevCon->Map(GPUBuffers[SBCreateResult.GPUBufferIndex], 0, SBCreateResult.mapMode, 0, &mappedData)))
	{
		result.data = mappedData.pData;
	}
	result.OutputBuffer = GPUBuffers[SBCreateResult.GPUBufferIndex];
	return result;
}

unsigned int ComputeShader::AddTexture(ID3D11ShaderResourceView* resource)
{
	resourceViews.push_back(resource);
	return resourceViews.size() -1;
}

void ComputeShader::SetTexture(unsigned int index, ID3D11ShaderResourceView* resource)
{
	resourceViews[index] = resource;
}

ID3D11ShaderResourceView* ComputeShader::GetTexture(unsigned int index){
	return resourceViews[index];
}

void ComputeShader::UnmapTexture(ID3D11Texture2D* mOutputBuffer) {
	d3d11DevCon->Unmap(mOutputBuffer, 0);
}

void ComputeShader::UnmapBuffer(ID3D11Buffer* mOutputBuffer)
{
	d3d11DevCon->Unmap(mOutputBuffer, 0);
}

void ComputeShader::Dispose()
{
	DX_RELEASE(CS);
	DX_RELEASE(CS_Buffer);
	ReleaseAllBuffers();
}

void ComputeShader::ReleaseGPUBuffers() {
	for (uint8_t i = 0; i < GPUBuffers.size(); ++i) DX_RELEASE(GPUBuffers[i]);
	GPUBuffers.clear();
}
void ComputeShader::ReleaseTextureBuffers()  {
	for (uint8_t i = 0; i < Textures.size(); ++i) DX_RELEASE(Textures[i]);
	for (uint8_t i = 0; i < resourceViews.size(); ++i) DX_RELEASE(resourceViews[i]);
	Textures.clear();
	resourceViews.clear();
}
void ComputeShader::ReleaseUAVs() 
{
	for (uint8_t i = 0; i < UAVs.size(); ++i) DX_RELEASE(UAVs[i]);
	UAVs.clear();
}

void ComputeShader::ReleaseGPUBuffer(const StructuredBufferHandle& handle) {

	//resourceViews[handle.resourceViewIndex]->Release();
}

void ComputeShader::RWReleaseUAV(const RWBufferHandle& handle) {
	resourceViews[handle.resourceViewIndex]->Release();
	UAVs[handle.UAV_Index]->Release();
}

void ComputeShader::ReleaseAllBuffers() {
	ReleaseGPUBuffers(); ReleaseTextureBuffers(); ReleaseUAVs();
}

ComputeShader::~ComputeShader()
{
	Dispose();
}