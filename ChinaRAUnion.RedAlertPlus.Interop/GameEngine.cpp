//
// Red Alert Plus
// GameEngine
// 作者：SunnyCase
// 创建时间：2016-05-24
//
#include "pch.h"
#include "GameEngine.h"
#include <windows.ui.xaml.media.dxinterop.h>

using namespace NS_RAP;
using namespace WRL;
using namespace Windows::UI::Core;
using namespace Windows::Graphics::Display;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;

namespace
{
	DXGI_MODE_ROTATION ComputeDisplayRotation(DisplayInformation^ displayInfo)
	{
		DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_UNSPECIFIED;

		// 注意: NativeOrientation 只能为 "Landscape" 或 "Portrait"，即使
		// DisplayOrientations 枚举具有其他值。
		switch (displayInfo->NativeOrientation)
		{
		case DisplayOrientations::Landscape:
			switch (displayInfo->CurrentOrientation)
			{
			case DisplayOrientations::Landscape:
				rotation = DXGI_MODE_ROTATION_IDENTITY;
				break;

			case DisplayOrientations::Portrait:
				rotation = DXGI_MODE_ROTATION_ROTATE270;
				break;

			case DisplayOrientations::LandscapeFlipped:
				rotation = DXGI_MODE_ROTATION_ROTATE180;
				break;

			case DisplayOrientations::PortraitFlipped:
				rotation = DXGI_MODE_ROTATION_ROTATE90;
				break;
			}
			break;

		case DisplayOrientations::Portrait:
			switch (displayInfo->CurrentOrientation)
			{
			case DisplayOrientations::Landscape:
				rotation = DXGI_MODE_ROTATION_ROTATE90;
				break;

			case DisplayOrientations::Portrait:
				rotation = DXGI_MODE_ROTATION_IDENTITY;
				break;

			case DisplayOrientations::LandscapeFlipped:
				rotation = DXGI_MODE_ROTATION_ROTATE270;
				break;

			case DisplayOrientations::PortraitFlipped:
				rotation = DXGI_MODE_ROTATION_ROTATE180;
				break;
			}
			break;
		}
		return rotation;
	}

	class ResourceResolver : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, NS_ENGINE::IResourceResovler>
	{
	public:
		ResourceResolver(IGameEngineResourceResolver^ resourceResolver)
			: _resourceResovler(resourceResolver)
		{

		}

		virtual Concurrency::task<std::vector<byte>> ResovleShader(const std::wstring &name)
		{
			auto res = _resourceResovler->ResolveShader(Platform::StringReference(name.c_str(), name.length()));
			return ReadStream(res);
		}

		virtual concurrency::task<NS_ENGINE::TileSetPackageContent> ResolveTileSetPackageFile(const std::wstring& name)
		{
			auto res = _resourceResovler->ResolveTileSetPackageFile(Platform::StringReference(name.c_str(), name.length()));
			auto tileSet = co_await ReadString(res->TileSet);
			auto image = co_await ReadStream(res->Image);
			auto extraImage = co_await ReadStream(res->ExtraImage);
			return{ tileSet, image,extraImage };
		}

		virtual concurrency::task<NS_ENGINE::BlobWithContentType> ResolveTexture(const std::wstring& name)
		{
			auto res = _resourceResovler->ResolveTexture(Platform::StringReference(name.c_str(), name.length()));
			return{ co_await ReadStream(res), std::wstring(res->ContentType->Begin(), res->ContentType->End()) };
		}
	private:
		Concurrency::task<std::vector<byte>> ReadStream(IRandomAccessStream^ stream)
		{
			auto dataReader = ref new DataReader(stream);
			auto len = co_await concurrency::create_task(dataReader->LoadAsync(stream->Size));
			std::vector<byte> data(len);
			dataReader->ReadBytes(Platform::ArrayReference<byte>(data.data(), data.size()));
			return std::move(data);
		}

		Concurrency::task<std::wstring> ReadString(IRandomAccessStream^ stream)
		{
			auto dataReader = ref new DataReader(stream);
			auto len = co_await concurrency::create_task(dataReader->LoadAsync(stream->Size));
			auto str = dataReader->ReadString(len);
			return std::wstring(str->Begin(), str->End());
		}
	private:
		IGameEngineResourceResolver^ _resourceResovler;
	};
}

GameEngine::GameEngine(IGameEngineResourceResolver^ resourceResovler)
	:_displayInfo(DisplayInformation::GetForCurrentView())
{
	auto resolver = Make<ResourceResolver>(resourceResovler);
	ThrowIfFailed(CreateEngine(&_engine, resolver.Get()));
	_engine->SetSwapChainChangedHandler([weak = Platform::WeakReference(this)](IDXGISwapChain* swapChain)
	{
		if (auto me = weak.Resolve<GameEngine>())
			if (auto panel = me->_swapChainPanel)
			{
				ComPtr<IDXGISwapChain> swapChainHolder(swapChain);
				panel->Dispatcher->RunAsync(CoreDispatcherPriority::High, ref new DispatchedHandler([=]
				{
					ComPtr<ISwapChainPanelNative> panelNative;
					ThrowIfFailed(reinterpret_cast<IInspectable*>(panel)->QueryInterface(IID_PPV_ARGS(&panelNative)));
					ThrowIfFailed(panelNative->SetSwapChain(swapChainHolder.Get()));
				}));
			}
	});

	_displayInfo->OrientationChanged += ref new Windows::Foundation::TypedEventHandler<Windows::Graphics::Display::DisplayInformation ^, Platform::Object ^>(this, &GameEngine::OnOrientationChanged);
	_displayInfo->DpiChanged += ref new Windows::Foundation::TypedEventHandler<Windows::Graphics::Display::DisplayInformation ^, Platform::Object ^>(this, &GameEngine::OnDpiChanged);
}

void GameEngine::SetSwapChainPanel(Windows::UI::Xaml::Controls::SwapChainPanel ^ panel)
{
	if (_swapChainPanel != nullptr)
	{
		_swapChainPanel->SizeChanged -= _onSizeChangedRegToken;
		_swapChainPanel->CompositionScaleChanged -= _onCompositionScaleChangedRegToken;
	}
	_swapChainPanel = panel;
	_onSizeChangedRegToken = panel->SizeChanged += ref new Windows::UI::Xaml::SizeChangedEventHandler(this, &GameEngine::OnSizeChanged);
	_onCompositionScaleChangedRegToken = panel->CompositionScaleChanged += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Xaml::Controls::SwapChainPanel ^, Platform::Object ^>(this, &GameEngine::OnCompositionScaleChanged);
	UpdateDisplayMetrices();
}

Windows::Foundation::IAsyncAction ^ GameEngine::InitializeAsync()
{
	return concurrency::create_async([this] {return _engine->InitializeAsync(); });
}

void GameEngine::UpdateDisplayMetrices()
{
	if (_swapChainPanel)
		_engine->UpdateDisplayMetrics(float(_swapChainPanel->ActualWidth), float(_swapChainPanel->ActualWidth), ComputeDisplayRotation(_displayInfo),
			_swapChainPanel->CompositionScaleX, _swapChainPanel->CompositionScaleY, _displayInfo->LogicalDpi);
}

void GameEngine::OnSizeChanged(Platform::Object ^sender, Windows::UI::Xaml::SizeChangedEventArgs ^e)
{
	UpdateDisplayMetrices();
}


void GameEngine::OnOrientationChanged(Windows::Graphics::Display::DisplayInformation ^sender, Platform::Object ^args)
{
	UpdateDisplayMetrices();
}


void GameEngine::OnDpiChanged(Windows::Graphics::Display::DisplayInformation ^sender, Platform::Object ^args)
{
	UpdateDisplayMetrices();
}


void GameEngine::OnCompositionScaleChanged(Windows::UI::Xaml::Controls::SwapChainPanel ^sender, Platform::Object ^args)
{
	UpdateDisplayMetrices();
}
