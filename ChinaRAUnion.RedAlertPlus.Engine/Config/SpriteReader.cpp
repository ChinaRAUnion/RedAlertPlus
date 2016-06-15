//
// Red Alert Plus Engine
// Sprite Reader
// 作者：SunnyCase
// 创建时间：2016-06-15
//
#include "pch.h"
#include "SpriteReader.h"
#include <rapidjson/document.h>

using namespace NS_ENGINE;
using namespace WRL;
using namespace rapidjson;

SpriteCoordinateReader::SpriteCoordinateReader(float imageWidth, float imageHeight, const std::wstring & json)
{
	GenericDocument<UTF16<>> document;
	document.Parse(json);


}
