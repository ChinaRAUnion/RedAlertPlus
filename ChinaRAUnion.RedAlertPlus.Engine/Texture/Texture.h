//
// Red Alert Plus Engine
// Texture
// 作者：SunnyCase
// 创建时间：2016-05-30
//
#pragma once
#include "../include/engine/Engine.h"
#include <d3d12.h>

DEFINE_NS_ENGINE

struct DataToUpload
{
	std::unique_ptr<byte[]> data;
	D3D12_SUBRESOURCE_DATA resData = {};
	WRL::ComPtr<ID3D12Resource> resForUpload;
};

class Texture
{
public:
	Texture()
		:_handle({ 0 }) {}
	Texture(ID3D12Resource* resource, D3D12_CPU_DESCRIPTOR_HANDLE handle, UINT width, UINT height)
		:_resource(resource), _handle(handle), _width(width), _height(height) {}

	const D3D12_CPU_DESCRIPTOR_HANDLE& get_Handle() const noexcept { return _handle; }
	DEFINE_PROPERTY_GET(Handle, const D3D12_CPU_DESCRIPTOR_HANDLE&);

	ID3D12Resource* get_Resource() const noexcept { return _resource.Get(); }
	DEFINE_PROPERTY_GET(Resource, ID3D12Resource*);

	bool IsValid() const noexcept { return _handle.ptr != 0; }

	UINT get_Width() const noexcept { return _width; }
	DEFINE_PROPERTY_GET(Width, UINT);
	UINT get_Height() const noexcept { return _height; }
	DEFINE_PROPERTY_GET(Height, UINT);

	std::unique_ptr<DataToUpload> dataForUpload;
private:
	WRL::ComPtr<ID3D12Resource> _resource;
	D3D12_CPU_DESCRIPTOR_HANDLE _handle;
	UINT _width, _height;
};

END_NS_ENGINE