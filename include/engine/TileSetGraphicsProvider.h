//
// Red Alert Plus Engine
// TileSetGraphicsProvider
// 作者：SunnyCase
// 创建时间：2016-06-12
//
#pragma once
#include "Engine.h"
#include <d3d11.h>

DEFINE_NS_ENGINE

interface DECLSPEC_UUID("7F1F41B5-9D32-4B9A-96D3-FFA901E97740") ITileSetGraphicsProvider : public IUnknown
{
	virtual concurrency::task<void> InitializeAsync(const std::wstring& tileSetName) = 0;
	virtual void CreatePickAnyUnit(uint32_t id, IDXGISurface** surface) = 0;
};


END_NS_ENGINE

extern "C"
{
	CRAU_ENGINE_API HRESULT STDMETHODCALLTYPE CreateTileSetGraphicsProvider(NS_ENGINE::ITileSetGraphicsProvider** provider, NS_ENGINE::IResourceResovler* resourceResolver);
}