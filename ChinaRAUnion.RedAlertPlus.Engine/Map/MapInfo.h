//
// Red Alert Plus Engine
// Map Info
// 作者：SunnyCase
// 创建时间：2016-05-30
//
#pragma once
#include "../../include/engine/common.h"
#include <rapidjson/document.h>

DEFINE_NS_ENGINE

struct MapInfo
{
	MapInfo(const rapidjson::GenericDocument<rapidjson::UTF16<>>& document);

	uint32_t width, height;
	std::wstring tileSet;
	std::vector<std::vector<uint32_t>> tiles;	// row major
};

END_NS_ENGINE