#pragma once
#include <wincodec.h>
#include "Palette.h"
#include "Shp.h"
#include "../../include/common.h"

class Converter
{
public:
	Converter(const Palette& palette, const Shp& shp);

	void Save(const std::wstring& fileName);
private:
	const Palette& _palette;
	const Shp& _shp;
	WRL::ComPtr<IWICImagingFactory> _wicFactory;
};