//
// Red Alert Plus Engine
// Map Info
// 作者：SunnyCase
// 创建时间：2016-05-30
//
#pragma once
#include "../../include/engine/engine.h"

DEFINE_NS_ENGINE

class TextureManager;

struct MapInfo
{
	MapInfo() {}
	MapInfo(const std::wstring& json);
	static concurrency::task<MapInfo> Generate(TextureManager& textureManager, const MapGenerateOptions & options, IResourceResovler * resourceResolver);

	uint32_t width, height;
	std::wstring tileSet;
	std::vector<std::vector<std::pair<uint32_t, uint32_t>>> tiles;	// row major
};

END_NS_ENGINE