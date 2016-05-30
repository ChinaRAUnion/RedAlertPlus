//
// Red Alert Plus Engine
// Texture
// ���ߣ�SunnyCase
// ����ʱ�䣺2016-05-30
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
	Texture(ID3D12Resource* resource, D3D12_CPU_DESCRIPTOR_HANDLE handle)
		:_resource(resource), _handle(handle) {}

	const D3D12_CPU_DESCRIPTOR_HANDLE& get_Handle() const noexcept { return _handle; }
	DEFINE_PROPERTY_GET(Handle, const D3D12_CPU_DESCRIPTOR_HANDLE&);

	ID3D12Resource* get_Resource() const noexcept { return _resource.Get(); }
	DEFINE_PROPERTY_GET(Resource, ID3D12Resource*);

	bool IsValid() const noexcept { return _handle.ptr != 0; }

	std::unique_ptr<DataToUpload> dataForUpload;
private:
	WRL::ComPtr<ID3D12Resource> _resource;
	D3D12_CPU_DESCRIPTOR_HANDLE _handle;
};

END_NS_ENGINE