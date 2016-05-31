//
// Red Alert Plus Engine
// TileSet Info
// 作者：SunnyCase
// 创建时间：2016-05-31
//
#pragma once
#include "../../include/engine/common.h"
#include "../Math/Collision.h"

DEFINE_NS_ENGINE

struct ExtraImage
{
	DirectX::XMFLOAT2 LeftTopUV, RightBottomUV;
	Tomato::Rect Rect = {};
};

struct Tile
{
	DirectX::XMFLOAT2 LeftTopUV, RightBottomUV;
	Tomato::Rect Rect = {};

	ExtraImage* ExtraImg = nullptr;
	DirectX::XMFLOAT2 ExtraImgOffset;
};

class TileSetInfo
{
public:
	TileSetInfo(const std::wstring& json, uint32_t imageWidth, uint32_t imageHeight, uint32_t extraImageWidth, uint32_t extraImageHeight);

	const Tile& FindTile(uint32_t id) const;

	uint32_t get_TileWidth() const noexcept { return _tileWidth; }
	DEFINE_PROPERTY_GET(TileWidth, uint32_t);
	uint32_t get_TileHeight() const noexcept { return _tileHeight; }
	DEFINE_PROPERTY_GET(TileHeight, uint32_t);
private:
	uint32_t _tileWidth, _tileHeight;
	std::vector<ExtraImage> _extraImages;
	std::vector<Tile> _tiles;
};

END_NS_ENGINE