//
// Red Alert Plus Engine
// Device Context
// 作者：SunnyCase
// 创建时间：2016-05-25
//
#pragma once
#include "../include/engine/Engine.h"
#include <d3d12.h>

DEFINE_NS_ENGINE

class DeviceContext
{
public:
	DeviceContext();

	void SetSwapChainChangedHandler(std::function<void(IDXGISwapChain*)> handler);
	void UpdateDisplayMetrics(float logicalWidth, float logicalHeight, DXGI_MODE_ROTATION rotation, float compositionScaleX, float compositionScaleY, float dpi);

	ID3D12Device* get_D3DDevice() const noexcept { return _d3dDevice.Get(); }
	DEFINE_PROPERTY_GET(D3DDevice, ID3D12Device*);

	DXGI_FORMAT get_BackBufferFormat() const noexcept { return _backBufferFormat; }
	DEFINE_PROPERTY_GET(BackBufferFormat, DXGI_FORMAT);

	DXGI_FORMAT get_DepthBufferFormat() const noexcept { return _depthBufferFormat; }
	DEFINE_PROPERTY_GET(DepthBufferFormat, DXGI_FORMAT);
private:
	void CreateDeviceIndependentResources();
	void CreateDeviceResoures();
	void CreateWindowSizeDependentResources();

	void WaitForGpu();
	void UpdateOutputSize();
	void OnSwapChainChanged(IDXGISwapChain* swapChain);

	UINT64& CurrentFence() { return _fenceValues.at(_currentFrame); }
private:
	static constexpr UINT FrameCount = 3;		// 三重缓冲
	DXGI_FORMAT _backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
	DXGI_FORMAT _depthBufferFormat = DXGI_FORMAT_D32_FLOAT;

	bool _deviceRemoved = false;

	Tomato::Core::Logger^ _logger;

	//Windows::UI::Xaml::Controls::SwapChainPanel^ _panel;
	WRL::ComPtr<IDXGIFactory4> _dxgiFactory;
	WRL::ComPtr<ID3D12Device> _d3dDevice;
	WRL::ComPtr<ID3D12CommandQueue> _commandQueue;
	WRL::ComPtr<ID3D12DescriptorHeap> _rtvDescHeap;
	UINT _rtvDescHandleSize;
	WRL::ComPtr<ID3D12DescriptorHeap> _dsvDescHeap;
	std::array<WRL::ComPtr<ID3D12CommandAllocator>, FrameCount> _commandAllocators;
	std::array<UINT64, FrameCount> _fenceValues{};
	std::array<WRL::ComPtr<ID3D12Resource>, FrameCount> _renderTargets;
	WRL::ComPtr<ID3D12Resource> _depthStencil;
	WRL::ComPtr<ID3D12Fence> _fence;
	WRL::ComPtr<IDXGISwapChain3> _swapChain;
	D3D12_VIEWPORT _screenViewport;

	size_t _currentFrame = 0;
	WRL::Wrappers::Event _fenceEvent;
	DXGI_MODE_ROTATION _rotation;
	float _compositionScaleX, _compositionScaleY;
	float _dpi;
	float _logicalWidth, _logicalHeight;
	float _outputWidth, _outputHeight;
	float _renderTargetWidth, _renderTargetHeight;

	std::function<void(IDXGISwapChain*)> _swapChainChangedHandler;
};

END_NS_ENGINE