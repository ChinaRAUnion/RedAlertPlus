//
// Red Alert Plus Engine
// Engine 实现
// 作者：SunnyCase
// 创建时间：2016-05-25
//
#pragma once
#include "../../include/engine/Engine.h"
#include "DeviceContext.h"

DEFINE_NS_ENGINE

class Engine : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, IEngine>
{
public:
	Engine(IResourceResovler* resourceResolver);

	virtual void SetSwapChainChangedHandler(std::function<void(IDXGISwapChain*)> handler);
	virtual void UpdateDisplayMetrics(float logicalWidth, float logicalHeight, DXGI_MODE_ROTATION rotation, float compositionScaleX, float compositionScaleY, float dpi);
private:
	WRL::ComPtr<IResourceResovler> _resourceResolver;
	DeviceContext _deviceContext;
};

END_NS_ENGINE