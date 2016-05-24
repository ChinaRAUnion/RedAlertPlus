//
// Red Alert Plus
// SwapChain Provider
// 作者：SunnyCase
// 创建时间：2016-05-24
//
#pragma once
#include "../include/common.h"
#include <d3d12.h>

DEFINE_NS_RAP

public ref class SwapChainProvider sealed
{
public:
	SwapChainProvider();

	void SetSwapChainPanel(Windows::UI::Xaml::Controls::SwapChainPanel^ panel);
private:
	void CreateDeviceIndependentResources();
	void CreateDeviceResoures();
private:
	Tomato::Core::Logger^ _logger;

	Windows::UI::Xaml::Controls::SwapChainPanel^ _panel;
	WRL::ComPtr<ID3D12Device> _d3dDevice;
	WRL::ComPtr<ID3D12CommandQueue> _commandQueue;
};

END_NS_RAP