//
// Red Alert Plus Engine
// Map Renderer
// 作者：SunnyCase
// 创建时间：2016-05-25
//
#pragma once
#include "DeviceContext.h"
#include "IDeviceDependentResourcesContainer.h"
#include "IRenderable.h"

DEFINE_NS_ENGINE

class MapRenderer : public IDeviceDependentResourcesContainer, public IRenderable
{
public:
	MapRenderer(DeviceContext& deviceContext);

	virtual concurrency::task<void> CreateDeviceDependentResources(IResourceResovler* resourceResolver);
	virtual concurrency::task<void> CreateWindowSizeDependentResources(IResourceResovler* resourceResolver);
	virtual void UploadGpuResource(std::vector<WRL::ComPtr<IUnknown>>& resourcesWaitForUpload);
	virtual void Update();
	virtual void Render();
private:
	DeviceContext& _deviceContext;
	WRL::ComPtr<ID3D12RootSignature> _rootSignature;
	WRL::ComPtr<ID3D12PipelineState> _pipelineState;
	WRL::ComPtr<ID3D12Resource> _vertexBuffer;
	WRL::ComPtr<ID3D12Resource> _indexBuffer;
	WRL::ComPtr<ID3D12GraphicsCommandList> _commandList;

	D3D12_VERTEX_BUFFER_VIEW _vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW _indexBufferView;
	D3D12_RECT _scissorRect;
};

END_NS_ENGINE