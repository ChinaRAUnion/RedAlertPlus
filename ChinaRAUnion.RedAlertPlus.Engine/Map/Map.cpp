//
// Red Alert Plus Engine
// Map
// 作者：SunnyCase
// 创建时间：2016-05-30
//
#include "pch.h"
#include "Map.h"

using namespace NS_ENGINE;
using namespace WRL;

Map::Map(DeviceContext& deviceContext, const std::wstring & mapName)
	:_deviceContext(deviceContext), _mapRender(deviceContext), _mapName(mapName)
{
}

Map::Map(DeviceContext & deviceContext, const MapGenerateOptions & options)
	: _deviceContext(deviceContext), _mapRender(deviceContext), _mapGenOptions(options)
{
}

concurrency::task<void> Map::InitializeAsync(IResourceResovler * resourceResolver)
{
	if (!_mapName.empty())
	{
		auto mapData = co_await resourceResolver->ResolveMap(_mapName);
		_mapInfo = std::make_shared<MapInfo>(mapData);
	}
	else
		_mapInfo = std::make_shared<MapInfo>(co_await MapInfo::Generate(_deviceContext.TextureManager, _mapGenOptions, resourceResolver));

	_mapRender.SetMap(_mapInfo);
}

concurrency::task<void> Map::CreateDeviceDependentResources(IResourceResovler * resourceResolver)
{
	co_await _mapRender.CreateDeviceDependentResources(resourceResolver);
}

concurrency::task<void> Map::CreateWindowSizeDependentResources(IResourceResovler * resourceResolver)
{
	co_await _mapRender.CreateWindowSizeDependentResources(resourceResolver);
}

void Map::UploadGpuResource(std::vector<WRL::ComPtr<IUnknown>>& resourcesWaitForUpload)
{
	_mapRender.UploadGpuResource(resourcesWaitForUpload);
}

void Map::Update()
{
	_currentMapMargin.x += _mapScrollSpeed.x;
	_currentMapMargin.y += _mapScrollSpeed.y;

	_currentMapMargin = _mapRender.SetMapMargin(_currentMapMargin);
	_mapRender.Update();
}

void Map::Render()
{
	_mapRender.Render();
}

void Map::SetMapScrollSpeed(float x, float y)
{
	_mapScrollSpeed = { x, y };
}
