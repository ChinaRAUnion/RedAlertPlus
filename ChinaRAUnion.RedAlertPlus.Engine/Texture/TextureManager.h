//
// Red Alert Plus Engine
// Texture Manager
// 作者：SunnyCase
// 创建时间：2016-05-30
//
#pragma once
#include "../include/engine/Engine.h"
#include <d3d12.h>
#include <unordered_map>
#include "Texture.h"
#pragma once

DEFINE_NS_ENGINE

class DeviceContext;

class TextureManager
{
public:
	TextureManager(DeviceContext& deviceContext);
	void Initialize();

	static const UINT HandlesPerHeap = 65536;

	Texture CreateTexture2D(const byte* data, size_t size, const std::wstring& contentType);
	void Upload(Texture& texture, ID3D12GraphicsCommandList* commandList, std::vector<WRL::ComPtr<IUnknown>>& resourcesWaitForUpload);
	ID3D12DescriptorHeap* GetHeap() const noexcept { return _srvDescHeap.Get(); }
private:
	size_t BitsPerPixel(WICPixelFormatGUID& guid);
	std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> AllocateHandle();
private:
	DeviceContext& _deviceContext;
	WRL::ComPtr<IWICImagingFactory> _wicFactory;

	UINT _srvDescHandleSize;
	WRL::ComPtr<ID3D12DescriptorHeap> _srvDescHeap;
	size_t _nextAvailOffset = 0;
};

END_NS_ENGINE