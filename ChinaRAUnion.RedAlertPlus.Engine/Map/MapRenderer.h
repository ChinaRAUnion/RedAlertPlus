//
// Red Alert Plus Engine
// Map Renderer
// ���ߣ�SunnyCase
// ����ʱ�䣺2016-05-25
//
#pragma once
#include "DeviceContext.h"
#include "IDeviceDependentResourcesContainer.h"

DEFINE_NS_ENGINE

class MapRenderer : public IDeviceDependentResourcesContainer
{
public:
	MapRenderer(DeviceContext& deviceContext);

	virtual concurrency::task<void> CreateDeviceDependentResources(IResourceResovler* resourceResolver);
	virtual void UploadGpuResource(std::vector<WRL::ComPtr<IUnknown>>& resourcesWaitForUpload);
private:
	DeviceContext& _deviceContext;
	WRL::ComPtr<ID3D12RootSignature> _rootSignature;
	WRL::ComPtr<ID3D12PipelineState> _pipelineState;
	WRL::ComPtr<ID3D12Resource> _vertexBuffer;
};

END_NS_ENGINE