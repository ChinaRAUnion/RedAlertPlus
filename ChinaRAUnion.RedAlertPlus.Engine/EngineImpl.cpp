//
// Red Alert Plus Engine
// Engine 实现
// 作者：SunnyCase
// 创建时间：2016-05-25
//
#include "pch.h"
#include "EngineImpl.h"
#include <sstream>

using namespace concurrency;

HRESULT STDMETHODCALLTYPE CreateEngine(NS_ENGINE::IEngine** engine, NS_ENGINE::IResourceResovler* resourceResolver)
{
	try
	{
		ARGUMENT_NOTNULL_HR(engine);
		*engine = WRL::Make<NS_ENGINE::Engine>(resourceResolver).Detach();
		return S_OK;
	}
	CATCH_ALL();
}

using namespace NS_ENGINE;
using namespace WRL;

Engine::Engine(IResourceResovler* resourceResolver)
	:_resourceResolver(resourceResolver)
{

}

void Engine::SetSwapChainChangedHandler(std::function<void(IDXGISwapChain*)> handler)
{
	_deviceContext.SetSwapChainChangedHandler(std::move(handler));
}

void Engine::UpdateDisplayMetrics(float logicalWidth, float logicalHeight, DXGI_MODE_ROTATION rotation, float compositionScaleX, float compositionScaleY, float dpi)
{
	_deviceContext.UpdateDisplayMetrics(logicalWidth, logicalHeight, rotation, compositionScaleX, compositionScaleY, dpi);
	CreateWindowSizeDependentResources();
}

concurrency::task<void> Engine::InitializeAsync()
{
	if (!_mapRender)
	{
		_mapRender = std::make_unique<MapRenderer>(_deviceContext);
		_resourceContainers.emplace_back(_mapRender.get());
		_renderables.emplace_back(_mapRender.get());
	}
	for (auto&& container : _resourceContainers)
		co_await container->CreateDeviceDependentResources(_resourceResolver.Get());

	std::vector<ComPtr<IUnknown>> resourcesWaitForUpload;
	for (auto&& container : _resourceContainers)
		container->UploadGpuResource(resourcesWaitForUpload);
	_deviceContext.WaitForGpu();
}

void Engine::Render()
{
	for (auto&& renderable : _renderables)
		renderable->Update();
	for (auto&& renderable : _renderables)
		renderable->Render();
	_deviceContext.Present();
}

concurrency::task<void> Engine::CreateWindowSizeDependentResources()
{
	for (auto&& container : _resourceContainers)
		co_await container->CreateWindowSizeDependentResources(_resourceResolver.Get());
}
