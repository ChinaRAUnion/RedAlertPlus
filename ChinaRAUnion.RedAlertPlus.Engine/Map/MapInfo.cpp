//
// Red Alert Plus Engine
// Map Info
// 作者：SunnyCase
// 创建时间：2016-05-30
//
#include "pch.h"
#include "MapInfo.h"
#include <rapidjson/document.h>
#include "TileSetInfo.h"
#include "../Texture/TextureManager.h"

using namespace NS_ENGINE;
using namespace WRL;
using namespace rapidjson;

MapInfo::MapInfo(const std::wstring& json)
{
	GenericDocument<UTF16<>> document;
	document.Parse(json);
	width = document[L"width"].GetUint();
	height = document[L"height"].GetUint();
	tileSet = document[L"tileSet"].GetString();

	tiles.resize(height);

	auto rowIt = tiles.begin();
	for (auto&& rowData : document[L"tiles"].GetArray())
	{
		rowIt->resize(width);
		auto columnIt = rowIt->begin();
		for (auto&& columnData : rowData.GetArray())
		{
			auto&& pair = columnData.GetArray();
			*columnIt = std::make_pair(pair[0].GetUint(), pair[1].GetUint());
			++columnIt;
		}
		++rowIt;
	}
}

concurrency::task<MapInfo> MapInfo::Generate(TextureManager& textureManager, const MapGenerateOptions & options, IResourceResovler * resourceResolver)
{
	auto tileSetRes = co_await resourceResolver->ResolveTileSetPackageFile(options.tileSetName);
	auto imageTex = textureManager.GetTexutre2DInfo(tileSetRes.Image.data(), tileSetRes.Image.size(), L"image/png");
	auto extImageTex = textureManager.GetTexutre2DInfo(tileSetRes.ExtraImage.data(), tileSetRes.ExtraImage.size(), L"image/png");
	TileSetInfo tileSet(tileSetRes.TileSet, imageTex.Width, imageTex.Height, extImageTex.Width, extImageTex.Height);

	MapInfo mapInfo;
	mapInfo.width = options.width;
	mapInfo.height = options.height;
	mapInfo.tileSet = options.tileSetName;
	mapInfo.tiles.reserve(mapInfo.height);
	
	const auto& clearTids = tileSet.FindPickAnyUnit(0).Tiles;
	const auto tidSize = clearTids.size();
	for (size_t row = 0; row < mapInfo.height; row++)
	{
		std::vector<std::pair<uint32_t, uint32_t>> rowContent;
		rowContent.reserve(mapInfo.width);
		for (size_t column = 0; column < mapInfo.width; column++)
		{
			auto tid = clearTids[rand() % tidSize];
			rowContent.emplace_back(tid, 0);
		}
		mapInfo.tiles.emplace_back(rowContent);
	}

	return mapInfo;
}
