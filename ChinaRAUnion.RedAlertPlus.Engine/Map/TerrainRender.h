//
// Red Alert Plus Engine
// Terrain Renderer
// 作者：SunnyCase
// 创建时间：2016-05-30
//
#pragma once
#include "DeviceContext.h"
#include "IDeviceDependentResourcesContainer.h"
#include "IRenderable.h"
#include "MapInfo.h"

DEFINE_NS_ENGINE

class TerrainRender : public IDeviceDependentResourcesContainer, public IRenderable
{
public:
	TerrainRender(DeviceContext& deviceContext);

	virtual concurrency::task<void> CreateDeviceDependentResources(IResourceResovler* resourceResolver);
	virtual concurrency::task<void> CreateWindowSizeDependentResources(IResourceResovler* resourceResolver);
	virtual void UploadGpuResource(std::vector<WRL::ComPtr<IUnknown>>& resourcesWaitForUpload);
	virtual void Update();
	virtual void Render();

	void SetMap(std::shared_ptr<MapInfo> mapInfo);
private:
	DeviceContext& _deviceContext;
	std::shared_ptr<MapInfo> _mapInfo;
	WRL::ComPtr<ID3D12RootSignature> _rootSignature;
	WRL::ComPtr<ID3D12PipelineState> _pipelineState;
	WRL::ComPtr<ID3D12Resource> _vertexBuffer;
	WRL::ComPtr<ID3D12Resource> _indexBuffer;
	WRL::ComPtr<ID3D12GraphicsCommandList> _commandList;

	Texture _tileSetImageTex, _tileSetExtraImageTex;

	D3D12_VERTEX_BUFFER_VIEW _vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW _indexBufferView;
	D3D12_RECT _scissorRect;
};

END_NS_ENGINE