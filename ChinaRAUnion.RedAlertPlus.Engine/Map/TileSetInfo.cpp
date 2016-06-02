//
// Red Alert Plus Engine
// TileSet Info
// 作者：SunnyCase
// 创建时间：2016-05-31
//
#include "pch.h"
#include "TileSetInfo.h"
#include <rapidjson/document.h>

using namespace NS_ENGINE;
using namespace WRL;
using namespace rapidjson;

TileSetInfo::TileSetInfo(const std::wstring & json, uint32_t imageWidth, uint32_t imageHeight, uint32_t extraImageWidth, uint32_t extraImageHeight)
{
	GenericDocument<UTF16<>> document;
	document.Parse(json);

	_tileWidth = document[L"tilewidth"].GetUint();
	_tileHeight = document[L"tileheight"].GetUint();

	for (auto&& extraImage : document[L"extraimages"].GetArray())
	{
		ExtraImage image;
		auto x = float(extraImage[L"x"].GetUint());
		auto y = float(extraImage[L"y"].GetUint());
		auto width = float(extraImage[L"width"].GetUint());
		auto height = float(extraImage[L"height"].GetUint());
		image.LeftTopUV = { x / extraImageWidth, y / extraImageHeight };
		image.RightBottomUV = { (x + width) / extraImageWidth, (y + height) / extraImageHeight };
		image.Rect.Width = width;
		image.Rect.Height = height;
		_extraImages.emplace_back(image);
	}

	uint32_t cntX = 0, cntY = 0;
	for (auto&& jTile : document[L"tiles"].GetArray())
	{
		Tile tile;
		if (jTile.HasMember(L"extraimage"))
		{
			auto&& ref = jTile[L"extraimage"];
			if (ref.IsObject())
			{
				tile.ExtraImg = &_extraImages[ref[L"extraimage"].GetUint()];

				auto&& offset = ref[L"offset"];
				tile.ExtraImgOffset = { float(offset[L"x"].GetInt()), float(-offset[L"y"].GetInt() + int(_tileHeight)) };
			}
		}
		tile.Rect.Width = _tileWidth;
		tile.Rect.Height = _tileHeight;
		tile.LeftTopUV = { float(cntX) / imageWidth, float(cntY) / imageHeight };
		tile.RightBottomUV = { float(cntX + _tileWidth) / imageWidth, float(cntY + _tileHeight) / imageHeight };
		_tiles.emplace_back(tile);

		cntX += _tileWidth;
		if (cntX + _tileWidth > imageWidth)
		{
			cntY += _tileHeight;
			cntX = 0;
		}
	}

	for (auto&& jPAU : document[L"pickanytileunits"].GetArray())
	{
		PickAnyUnit pau;
		pau.Category = jPAU[L"category"].GetString();
		for (auto&& tile : jPAU[L"tiles"].GetArray())
			pau.Tiles.emplace_back(tile.GetUint());
		_pickAnyUnits.emplace_back(pau);
	}
}

const Tile & TileSetInfo::FindTile(uint32_t id) const
{
	if (_tiles.empty())
		ThrowAlways(L"No tile found in tileset.");
	return id < _tiles.size() ? _tiles[id] : _tiles.front();
}

const PickAnyUnit & TileSetInfo::FindPickAnyUnit(uint32_t id) const
{
	if (_pickAnyUnits.empty())
		ThrowAlways(L"No pickanyunit found in tileset.");
	return id < _pickAnyUnits.size() ? _pickAnyUnits[id] : _pickAnyUnits.front();
}
