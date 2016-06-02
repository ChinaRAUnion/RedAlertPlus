//
// Red Alert Plus Engine
// Map Renderer
// 作者：SunnyCase
// 创建时间：2016-05-25
//
#include "pch.h"
#include "MapRenderer.h"

using namespace NS_ENGINE;
using namespace WRL;

MapRenderer::MapRenderer(DeviceContext & deviceContext)
	:_deviceContext(deviceContext), _terrainRender(deviceContext)
{
}

concurrency::task<void> MapRenderer::CreateDeviceDependentResources(IResourceResovler* resourceResolver)
{
	co_await _terrainRender.CreateDeviceDependentResources(resourceResolver);
}

concurrency::task<void> MapRenderer::CreateWindowSizeDependentResources(IResourceResovler * resourceResolver)
{
	co_await _terrainRender.CreateWindowSizeDependentResources(resourceResolver);
}

void MapRenderer::UploadGpuResource(std::vector<WRL::ComPtr<IUnknown>>& resourcesWaitForUpload)
{
	_terrainRender.UploadGpuResource(resourcesWaitForUpload);
}

void MapRenderer::Update()
{
	_terrainRender.Update();
}

void MapRenderer::Render()
{
	_terrainRender.Render();
}

void MapRenderer::SetMap(std::shared_ptr<MapInfo> mapInfo)
{
	ThrowIfNot(mapInfo, L"Map cannot be nullptr.");
	_terrainRender.SetMap(mapInfo);
}

DirectX::XMFLOAT2 MapRenderer::SetMapMargin(const DirectX::XMFLOAT2 & margin)
{
	return _terrainRender.SetMapMargin(margin);
}
