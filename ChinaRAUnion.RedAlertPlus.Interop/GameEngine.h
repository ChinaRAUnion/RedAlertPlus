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
	property Windows::Storage::Streams::IInputStream^ TileSet
	{
		Windows::Storage::Streams::IInputStream^ get();
	}

	property Windows::Storage::Streams::IInputStream^ Image
	{
		Windows::Storage::Streams::IInputStream^ get();
	}

	property Windows::Storage::Streams::IInputStream^ ExtraImage
	{
		Windows::Storage::Streams::IInputStream^ get();
	}
};

public interface class IGameEngineResourceResolver
{
	Windows::Storage::Streams::IRandomAccessStream^ ResolveShader(Platform::String^ name);
	ITileSetPackageContent^ ResolveTileSetPackageFile(Platform::String^ name);
	Windows::Storage::Streams::IRandomAccessStreamWithContentType^ ResolveTexture(Platform::String^ name);
	Windows::Storage::Streams::IRandomAccessStream^ ResolveMap(Platform::String^ name);
};

public interface class IGameEngineRulesResolver
{
	property Windows::Foundation::Collections::IMapView<Platform::String^, Platform::Object^>^ Infantry
	{
		Windows::Foundation::Collections::IMapView<Platform::String^, Platform::Object^>^ get();
	}
};

public enum class GameMode
{
	Play,
	MapEdit
};

public ref class GameMapGenerateOptions sealed
{
public:
	property uint32_t Width
	{
		uint32_t get() { return _width; }
		void set(uint32_t value) { _width = value; }
	}
	
	property uint32_t Height
	{
		uint32_t get() { return _height; }
		void set(uint32_t value) { _height = value; }
	}

	property Platform::String^ TileSetName
	{
		Platform::String^ get() { return _tileSetName; }
		void set(Platform::String^ value) { _tileSetName = value; }
	}
private:
	uint32_t _width, _height;
	Platform::String^ _tileSetName;
};

[Windows::Foundation::Metadata::WebHostHiddenAttribute]
public ref class GameEngine sealed
{
public:
	GameEngine(IGameEngineResourceResolver^ resourceResolver, IGameEngineRulesResolver^ rulesResolver);

	property GameMode Mode
	{
		GameMode get() { return _gameMode; }
		void set(GameMode value) { _gameMode = value; }
	}

	void SetSwapChainPanel(Windows::UI::Xaml::Controls::SwapChainPanel^ panel);

	void UseMap(Platform::String^ mapName);
	void GenerateMap(GameMapGenerateOptions^ options);
	Windows::Foundation::IAsyncAction^ InitializeAsync();
	void StartRenderLoop();

	void OnPointerMoved(Windows::Foundation::Size uiSize, Windows::UI::Input::PointerPoint^ point);
	void OnPointerPressed(Windows::Foundation::Size uiSize, Windows::UI::Input::PointerPoint^ point);
	void OnPointerReleased(Windows::Foundation::Size uiSize, Windows::UI::Input::PointerPoint^ point);
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
	GameMode _gameMode = GameMode::Play;

	Windows::Foundation::Point _lastRBPressedPoint;
	std::mutex _deviceLock;
};

END_NS_RAP