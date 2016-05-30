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
#include "TerrainRender.h"
#include "MapInfo.h"

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

	void SetMap(std::shared_ptr<MapInfo> mapInfo);
private:
	DeviceContext& _deviceContext;
	TerrainRender _terrainRender;
};

END_NS_ENGINE