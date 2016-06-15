//
// Red Alert Plus Engine
// Sprite Reader
// 作者：SunnyCase
// 创建时间：2016-06-15
//
#pragma once
#include "../../include/engine/common.h"
#include "../Math/Collision.h"

DEFINE_NS_ENGINE

#pragma pack(push, 1)
struct SpriteFrame
{
	DirectX::XMFLOAT2 LeftTopUV;
	DirectX::XMFLOAT2 RightBottomUV;
	DirectX::XMFLOAT4X4 FrameTransform;
};
#pragma pack(pop)

struct SpriteCoordinateReader
{
	SpriteCoordinateReader(float imageWidth, float imageHeight, const std::wstring& json);

	std::vector<SpriteFrame> Frames;
};

struct SpriteSequenceReader
{
	SpriteSequenceReader(const std::wstring& json);
};

END_NS_ENGINE