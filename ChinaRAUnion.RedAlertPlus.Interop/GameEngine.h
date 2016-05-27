//
// Red Alert Plus
// GameEngine
// 作者：SunnyCase
// 创建时间：2016-05-24
//
#pragma once
#include "../include/common.h"
#include "../include/engine/Engine.h"

DEFINE_NS_RAP

public interface class ITileSetPackageContent
{
	property Windows::Storage::Streams::IRandomAccessStream^ TileSet
	{
		Windows::Storage::Streams::IRandomAccessStream^ get();
	}

	property Windows::Storage::Streams::IRandomAccessStream^ Image
	{
		Windows::Storage::Streams::IRandomAccessStream^ get();
	}

	property Windows::Storage::Streams::IRandomAccessStream^ ExtraImage
	{
		Windows::Storage::Streams::IRandomAccessStream^ get();
	}
};

public interface class IGameEngineResourceResolver
{
	Windows::Storage::Streams::IRandomAccessStream^ ResolveShader(Platform::String^ name);
	ITileSetPackageContent^ ResolveTileSetPackageFile(Platform::String^ name);
	Windows::Storage::Streams::IRandomAccessStreamWithContentType^ ResolveTexture(Platform::String^ name);
};

[Windows::Foundation::Metadata::WebHostHiddenAttribute]
public ref class GameEngine sealed
{
public:
	GameEngine(IGameEngineResourceResolver^ resourceResovler);

	void SetSwapChainPanel(Windows::UI::Xaml::Controls::SwapChainPanel^ panel);
	Windows::Foundation::IAsyncAction^ InitializeAsync();
	void StartRenderLoop();
private:
	void UpdateDisplayMetrices();
	void OnSizeChanged(Platform::Object ^sender, Windows::UI::Xaml::SizeChangedEventArgs ^e);
	void OnOrientationChanged(Windows::Graphics::Display::DisplayInformation ^sender, Platform::Object ^args);
	void OnDpiChanged(Windows::Graphics::Display::DisplayInformation ^sender, Platform::Object ^args);
	void OnCompositionScaleChanged(Windows::UI::Xaml::Controls::SwapChainPanel ^sender, Platform::Object ^args);
private:
	WRL::ComPtr<Engine::IEngine> _engine;
	Windows::UI::Xaml::Controls::SwapChainPanel^ _swapChainPanel;
	Windows::Graphics::Display::DisplayInformation^ _displayInfo;
	Windows::Foundation::EventRegistrationToken _onSizeChangedRegToken, _onCompositionScaleChangedRegToken;
};

END_NS_RAP