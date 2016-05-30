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
using namespace rapidjson;

Map::Map(DeviceContext& deviceContext, const std::wstring & mapName)
	:_mapRender(deviceContext), _mapName(mapName)
{
}

concurrency::task<void> Map::InitializeAsync(IResourceResovler * resourceResolver)
{
	auto mapData = co_await resourceResolver->ResolveMap(_mapName);
	GenericDocument<UTF16<>> document;
	document.Parse(mapData);
	_mapInfo = std::make_shared<MapInfo>(document);

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
	_mapRender.Update();
}

void Map::Render()
{
	_mapRender.Render();
}
