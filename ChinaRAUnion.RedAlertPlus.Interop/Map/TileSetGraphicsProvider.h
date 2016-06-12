//
// Red Alert Plus
// TileSetGraphicsProvider
// 作者：SunnyCase
// 创建时间：2016-06-12
//
#pragma once
#include "../GameEngine.h"
#include "../../include/engine/TileSetGraphicsProvider.h"

DEFINE_NS_RAP

public ref class TileSetGraphicsProvider sealed
{
public:
	static Windows::Foundation::IAsyncOperation<TileSetGraphicsProvider^>^ CreateFromTileSet(IGameEngineResourceResolver^ resourceResovler, Platform::String^ tileSetName);

	Windows::Foundation::IAsyncOperation<Windows::UI::Xaml::Media::ImageSource^>^ CreatePickAnyUnit(int id);
private:
	TileSetGraphicsProvider(IGameEngineResourceResolver^ resourceResovler);
	Windows::Foundation::IAsyncAction^ InitializeAsync(Platform::String^ tileSetName);
private:
	WRL::ComPtr<NS_ENGINE::ITileSetGraphicsProvider> _tsgProvider;
};

END_NS_RAP