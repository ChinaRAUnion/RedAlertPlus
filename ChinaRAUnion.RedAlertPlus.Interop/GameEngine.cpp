//
// Red Alert Plus
// GameEngine
// 作者：SunnyCase
// 创建时间：2016-05-24
//
#include "pch.h"
#include "GameEngine.h"
#include <windows.ui.xaml.media.dxinterop.h>
#include <sstream>
#include "ResourceResolver.h"

using namespace NS_RAP;
using namespace WRL;
using namespace Windows::UI::Core;
using namespace Windows::Graphics::Display;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;

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

void GameEngine::UseMap(Platform::String ^ mapName)
{
	_engine->UseMap({ mapName->Begin(), mapName->End() });
}

void GameEngine::GenerateMap(GameMapGenerateOptions ^ options)
{
	_engine->GenerateMap(
	{
		options->Width,
		options->Height,
		std::wstring(options->TileSetName->Begin(), options->TileSetName->End())
	});
}

Windows::Foundation::IAsyncAction ^ GameEngine::InitializeAsync()
{
	return concurrency::create_async([this] {return _engine->InitializeAsync(); });
}

void GameEngine::StartRenderLoop()
{
	// 创建一个将在后台线程上运行的任务。
	auto workItemHandler = ref new WorkItemHandler([this](IAsyncAction ^ action)
	{
		// 计算更新的帧并且在每个场消隐期呈现一次。
		while (action->Status == AsyncStatus::Started)
		{
			std::unique_lock<std::mutex> locker(_deviceLock);
			_engine->Render();
		}
	});

	// 在高优先级的专用后台线程上运行任务。
	ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
}

void GameEngine::OnPointerMoved(Windows::Foundation::Size uiSize, Windows::UI::Input::PointerPoint ^ point)
{
	// 快速卷动
	if (point->Properties->IsRightButtonPressed)
	{
		static double slideFactor = 0.3, maxSlideSpped = 10;
		const auto position = point->Position;

		auto leftMove = (position.X - _lastRBPressedPoint.X) * slideFactor;
		auto topMove = -(position.Y - _lastRBPressedPoint.Y) * slideFactor;
		_engine->SetMapScrollSpeed(leftMove < 0 ? std::max(-maxSlideSpped, leftMove) : std::min(maxSlideSpped, leftMove),
			topMove < 0 ? std::max(-maxSlideSpped, topMove) : std::min(maxSlideSpped, topMove));
	}
	// 普通卷动
	else
	{
		if (_gameMode == GameMode::Play)
		{
			static const double slideLength = 20, slideSpeed = 3;
			const auto position = point->Position;

			auto leftMove = position.X < slideLength ? -slideSpeed : 0;
			auto topMove = position.Y < slideLength ? slideSpeed : 0;
			auto rightMove = uiSize.Width - position.X < slideLength ? slideSpeed : 0;
			auto bottomMove = uiSize.Height - position.Y < slideLength ? -slideSpeed : 0;
			_engine->SetMapScrollSpeed(leftMove + rightMove, topMove + bottomMove);
		}
		else
			_engine->SetMapScrollSpeed(0, 0);
	}
}

void GameEngine::OnPointerPressed(Windows::Foundation::Size uiSize, Windows::UI::Input::PointerPoint ^ point)
{
	if (point->Properties->IsRightButtonPressed)
	{
		_lastRBPressedPoint = point->Position;
	}
}

void GameEngine::OnPointerReleased(Windows::Foundation::Size uiSize, Windows::UI::Input::PointerPoint ^ point)
{
	OnPointerMoved(uiSize, point);
}

void GameEngine::UpdateDisplayMetrices()
{
	std::unique_lock<std::mutex> locker(_deviceLock);
	if (_swapChainPanel)
	{
		_engine->UpdateDisplayMetrics(float(_swapChainPanel->ActualWidth), float(_swapChainPanel->ActualHeight), ComputeDisplayRotation(_displayInfo),
			_swapChainPanel->CompositionScaleX, _swapChainPanel->CompositionScaleY, _displayInfo->LogicalDpi);
	}
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
