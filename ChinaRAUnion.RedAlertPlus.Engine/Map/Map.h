//
// Red Alert Plus Engine
// Map
// 作者：SunnyCase
// 创建时间：2016-05-30
//
#pragma once
#include "../../include/engine/common.h"
#include "../IDeviceDependentResourcesContainer.h"
#include <ppltasks.h>
#include "MapRenderer.h"
#include "MapInfo.h"

DEFINE_NS_ENGINE

class Map : public IDeviceDependentResourcesContainer, public IRenderable
{
public:
	Map(DeviceContext& deviceContext, const std::wstring & mapName);

	concurrency::task<void> InitializeAsync(IResourceResovler * resourceResolver);
	virtual concurrency::task<void> CreateDeviceDependentResources(IResourceResovler* resourceResolver);
	virtual concurrency::task<void> CreateWindowSizeDependentResources(IResourceResovler* resourceResolver);
	virtual void UploadGpuResource(std::vector<WRL::ComPtr<IUnknown>>& resourcesWaitForUpload);

	virtual void Update();
	virtual void Render();

	void SetMapScrollSpeed(float x, float y);
private:
	MapRenderer _mapRender;
	std::wstring _mapName;
	std::shared_ptr<MapInfo> _mapInfo;
	DirectX::XMFLOAT2 _mapScrollSpeed;
	DirectX::XMFLOAT2 _currentMapMargin = {};
};

END_NS_ENGINE