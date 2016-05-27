//
// Red Alert Plus Engine
// Engine 实现
// 作者：SunnyCase
// 创建时间：2016-05-25
//
#pragma once
#include "../../include/engine/Engine.h"
#include "DeviceContext.h"
#include <Map/MapRenderer.h>

DEFINE_NS_ENGINE

class Engine : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, IEngine>
{
public:
	Engine(IResourceResovler* resourceResolver);

	virtual void SetSwapChainChangedHandler(std::function<void(IDXGISwapChain*)> handler);
	virtual void UpdateDisplayMetrics(float logicalWidth, float logicalHeight, DXGI_MODE_ROTATION rotation, float compositionScaleX, float compositionScaleY, float dpi);
	virtual concurrency::task<void> InitializeAsync();
	virtual void Render();
private:
	concurrency::task<void> CreateWindowSizeDependentResources();
private:
	WRL::ComPtr<IResourceResovler> _resourceResolver;
	DeviceContext _deviceContext;
	std::unique_ptr<MapRenderer> _mapRender;
	std::vector<IDeviceDependentResourcesContainer*> _resourceContainers;
	std::vector<IRenderable*> _renderables;
};

END_NS_ENGINE