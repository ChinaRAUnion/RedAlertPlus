//
// Red Alert Plus Engine
// Map Info
// 作者：SunnyCase
// 创建时间：2016-05-30
//
#include "pch.h"
#include "MapInfo.h"

using namespace NS_ENGINE;
using namespace WRL;

MapInfo::MapInfo(const rapidjson::GenericDocument<rapidjson::UTF16<>>& document)
	:width(document[L"width"].GetUint()),
	height(document[L"height"].GetUint()),
	tileSet(document[L"tileSet"].GetString()),
	tiles(height)
{
	auto rowIt = tiles.begin();
	for (auto&& rowData : document[L"tiles"].GetArray())
	{
		rowIt->resize(width);
		auto columnIt = rowIt->begin();
		for (auto&& columnData : rowData.GetArray())
		{
			*columnIt = columnData.GetUint();
			++columnIt;
		}
		++rowIt;
	}
}
