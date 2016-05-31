//
// Red Alert Plus Engine
// Map Info
// 作者：SunnyCase
// 创建时间：2016-05-30
//
#include "pch.h"
#include "MapInfo.h"
#include <rapidjson/document.h>

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
