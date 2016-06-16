//
// Red Alert Plus Engine
// Device Context
// 作者：SunnyCase
// 创建时间：2016-05-25
//
#pragma once
#include "../include/engine/Engine.h"
#include <d3d12.h>
#include <array>
#include "Texture/TextureManager.h"

DEFINE_NS_ENGINE

class DeviceContext
{
public:
	DeviceContext();

	void SetSwapChainChangedHandler(std::function<void(IDXGISwapChain*)> handler);
	void UpdateDisplayMetrics(float logicalWidth, float logicalHeight, DXGI_MODE_ROTATION rotation, float compositionScaleX, float compositionScaleY, float dpi);
	void CreateCommandList(UINT nodeMask, D3D12_COMMAND_LIST_TYPE type, ID3D12PipelineState* pInitialState, REFIID riid, void** ppCommandList);
	void WaitForGpu();

	ID3D12Device* get_D3DDevice() const noexcept { return _d3dDevice.Get(); }
	DEFINE_PROPERTY_GET(D3DDevice, ID3D12Device*);

	DXGI_FORMAT get_BackBufferFormat() const noexcept { return _backBufferFormat; }
	DEFINE_PROPERTY_GET(BackBufferFormat, DXGI_FORMAT);

	DXGI_FORMAT get_DepthBufferFormat() const noexcept { return _depthBufferFormat; }
	DEFINE_PROPERTY_GET(DepthBufferFormat, DXGI_FORMAT);

	ID3D12CommandAllocator* get_CurrentCommandAllocator() const noexcept { return _commandAllocators[_currentFrame].Get(); }
	DEFINE_PROPERTY_GET(CurrentCommandAllocator, ID3D12CommandAllocator*);

	const D3D12_VIEWPORT& get_ScreenViewport() const noexcept { return _screenViewport; }
	DEFINE_PROPERTY_GET(ScreenViewport, const D3D12_VIEWPORT&);

	ID3D12Resource* get_RenderTarget() const noexcept { return _renderTargets.at(_currentFrame).Get(); }
	DEFINE_PROPERTY_GET(RenderTarget, ID3D12Resource*);

	CD3DX12_CPU_DESCRIPTOR_HANDLE get_RenderTargetView() const
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(_rtvDescHeap->GetCPUDescriptorHandleForHeapStart(), _currentFrame, _rtvDescHandleSize);
	}
	DEFINE_PROPERTY_GET(RenderTargetView, CD3DX12_CPU_DESCRIPTOR_HANDLE);
	CD3DX12_CPU_DESCRIPTOR_HANDLE get_DepthStencilView() const
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(_dsvDescHeap->GetCPUDescriptorHandleForHeapStart());
	}
	DEFINE_PROPERTY_GET(DepthStencilView, CD3DX12_CPU_DESCRIPTOR_HANDLE);

	TextureManager& get_TextureManager() noexcept { return _textureManager; }
	DEFINE_PROPERTY_GET(TextureManager, TextureManager&);

	size_t get_CurrentFrameIndex() const noexcept { return _currentFrame; }
	DEFINE_PROPERTY_GET(CurrentFrameIndex, size_t);

	void ExecuteCommandList(ID3D12CommandList* commandList);
	void Present();

	void UploadResource(IUnknown* resource);
	WRL::ComPtr<ID3D12Resource> CreateVertexBuffer(ID3D12GraphicsCommandList* commandList, const void* data, size_t dataSize);

	static constexpr UINT FrameCount = 3;		// 三重缓冲
private:
	void CreateDeviceIndependentResources();
	void CreateDeviceResources();
	void CreateWindowSizeDependentResources();

	void UpdateOutputSize();
	void OnSwapChainChanged(IDXGISwapChain* swapChain);
	void MoveToNextFrame();

	UINT64& CurrentFence() { return _fenceValues.at(_currentFrame); }
private:
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
	std::vector<WRL::ComPtr<IUnknown>> _resourcesWaitForUpload;
	D3D12_VIEWPORT _screenViewport;
	::NS_ENGINE::TextureManager _textureManager;

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