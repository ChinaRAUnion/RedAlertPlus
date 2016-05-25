//
// Red Alert Plus Engine
// Map Renderer
// ���ߣ�SunnyCase
// ����ʱ�䣺2016-05-25
//
#pragma once
#include "DeviceContext.h"

DEFINE_NS_ENGINE

class MapRenderer
{
public:
	MapRenderer(DeviceContext& deviceContext);

	concurrency::task<void> CreateDeviceDependentResources(IResourceResovler* resourceResolver);
private:
	DeviceContext& _deviceContext;
	WRL::ComPtr<ID3D12RootSignature> _rootSignature;
	WRL::ComPtr<ID3D12PipelineState> _pipelineState;
};

END_NS_ENGINE