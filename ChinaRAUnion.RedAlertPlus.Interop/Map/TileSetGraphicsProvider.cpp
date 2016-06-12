//
// Red Alert Plus
// TileSetGraphicsProvider
// 作者：SunnyCase
// 创建时间：2016-06-12
//
#include "pch.h"
#include "TileSetGraphicsProvider.h"
#include "ResourceResolver.h"
#include <Windows.Graphics.DirectX.Direct3D11.interop.h>

using namespace NS_RAP;
using namespace WRL;
using namespace Windows::UI::Core;
using namespace Windows::Graphics::Display;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Windows::UI::Xaml::Media::Imaging;
using namespace Windows::Graphics::Imaging;
using namespace Windows::Graphics::DirectX::Direct3D11;

Windows::Foundation::IAsyncOperation<TileSetGraphicsProvider^>^ TileSetGraphicsProvider::CreateFromTileSet(IGameEngineResourceResolver^ resourceResovler, Platform::String^ tileSetName)
{
	return concurrency::create_async([=]()->concurrency::task<TileSetGraphicsProvider^> {
		auto provider = ref new TileSetGraphicsProvider(resourceResovler);
		co_await concurrency::create_task(provider->InitializeAsync(tileSetName));
		return provider;
	});
}

TileSetGraphicsProvider::TileSetGraphicsProvider(IGameEngineResourceResolver^ resourceResovler)
{
	auto resolver = Make<ResourceResolver>(resourceResovler);
	ThrowIfFailed(CreateTileSetGraphicsProvider(&_tsgProvider, resolver.Get()));
}

Windows::Foundation::IAsyncAction ^ TileSetGraphicsProvider::InitializeAsync(Platform::String^ tileSetName)
{
	return concurrency::create_async([=]
	{
		return _tsgProvider->InitializeAsync({ tileSetName->Begin(), tileSetName->End() });
	});
}

Windows::Foundation::IAsyncOperation<Windows::UI::Xaml::Media::ImageSource^>^ TileSetGraphicsProvider::CreatePickAnyUnit(int id)
{
	return concurrency::create_async([=]()->concurrency::task<Windows::UI::Xaml::Media::ImageSource^> {
		auto bmpSrc = ref new SoftwareBitmapSource();
		ComPtr<IDXGISurface> dxgiSurface;
		_tsgProvider->CreatePickAnyUnit(uint32_t(id), &dxgiSurface);
		if (dxgiSurface)
		{
			auto bmp = co_await concurrency::create_task(SoftwareBitmap::CreateCopyFromSurfaceAsync(CreateDirect3DSurface(dxgiSurface.Get()), BitmapAlphaMode::Premultiplied));
			co_await concurrency::create_task(bmpSrc->SetBitmapAsync(bmp));
		}
		return bmpSrc;
	});
}
