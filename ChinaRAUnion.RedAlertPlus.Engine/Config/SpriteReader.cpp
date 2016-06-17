//
// Red Alert Plus Engine
// Sprite Reader
// ���ߣ�SunnyCase
// ����ʱ�䣺2016-06-15
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

	{
		auto&& jSize = document[L"size"];
		Width = jSize[L"width"].GetUint();
		Height = jSize[L"height"].GetUint();
	}
}

SpriteSequenceReader::SpriteSequenceReader(const std::wstring & json)
{
	GenericDocument<UTF16<>> document;
	document.Parse(json);
}
