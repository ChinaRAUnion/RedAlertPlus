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

void Engine::UseMap(const std::wstring& mapName)
{
	if (_map)
	{
		_resourceContainers.erase(std::find(_resourceContainers.begin(), _resourceContainers.end(), _map.get()));
		_renderables.erase(std::find(_renderables.begin(), _renderables.end(), _map.get()));
	}
	_map = std::make_unique<Map>(_deviceContext, mapName);
	_resourceContainers.emplace_back(_map.get());
	_renderables.emplace_back(_map.get());
}

void Engine::GenerateMap(const MapGenerateOptions& options)
{
	if (_map)
	{
		_resourceContainers.erase(std::find(_resourceContainers.begin(), _resourceContainers.end(), _map.get()));
		_renderables.erase(std::find(_renderables.begin(), _renderables.end(), _map.get()));
	}
	_map = std::make_unique<Map>(_deviceContext, options);
	_resourceContainers.emplace_back(_map.get());
	_renderables.emplace_back(_map.get());
}

concurrency::task<void> Engine::InitializeAsync()
{
	if (_map) co_await _map->InitializeAsync(_resourceResolver.Get());

	for (auto&& container : _resourceContainers)
		co_await container->CreateDeviceDependentResources(_resourceResolver.Get());

	std::vector<ComPtr<IUnknown>> resourcesWaitForUpload;
	for (auto&& container : _resourceContainers)
		container->UploadGpuResource(resourcesWaitForUpload);
	_deviceContext.WaitForGpu();
	CreateWindowSizeDependentResources();
}

void Engine::Render()
{
	for (auto&& renderable : _renderables)
		renderable->Update();
	for (auto&& renderable : _renderables)
		renderable->Render();
	_deviceContext.Present();
}

void Engine::SetMapScrollSpeed(float x, float y)
{
	if (_map) _map->SetMapScrollSpeed(x, y);
}

concurrency::task<void> Engine::CreateWindowSizeDependentResources()
{
	for (auto&& container : _resourceContainers)
		co_await container->CreateWindowSizeDependentResources(_resourceResolver.Get());
}
