//
// Red Alert Plus Engine
// Device Context
// 作者：SunnyCase
// 创建时间：2016-05-25
//
#include "pch.h"
#include "DeviceContext.h"
#include "../include/d3dx12.h"
#include <sstream>

using namespace NS_ENGINE;
using namespace WRL;

namespace
{

	// 此方法获取支持 Direct3D 12 的第一个可用硬件适配器。
	// 如果找不到此类适配器，则 *ppAdapter 将设置为 nullptr。
	void GetHardwareAdapter(IDXGIFactory1* dxgiFactory, IDXGIAdapter1** ppAdapter)
	{
		ComPtr<IDXGIAdapter1> adapter;
		*ppAdapter = nullptr;

		for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != dxgiFactory->EnumAdapters1(adapterIndex, &adapter); adapterIndex++)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				// 不要选择基本呈现驱动程序适配器。
				continue;
			}

			// 检查适配器是否支持 Direct3D 12，但不要创建
			// 仍为实际设备。
			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
			{
				break;
			}
		}

		*ppAdapter = adapter.Detach();
	}

	void DumpAdapterInfo(NS_CORE::Logger^ logger, IDXGIAdapter* adapter)
	{
		if (adapter)
		{
			DXGI_ADAPTER_DESC desc;
			ThrowIfFailed(adapter->GetDesc(&desc));

			std::wstringstream ss;
			ss << std::endl << L"Adapter Information: ";
			ss << std::endl << L"Device Id: " << desc.DeviceId;
			ss << std::endl << L"Description: " << desc.Description;
			ss << std::endl << L"Dedicated Video Memory: " << desc.DedicatedVideoMemory << L" Bytes";
			ss << std::endl << L"Shared System Memory: " << desc.SharedSystemMemory << L" Bytes";
			auto message = ss.str();

			logger->Information(Platform::StringReference(message.c_str(), message.length()));
		}
	}

	// 将使用与设备无关的像素(DIP)表示的长度转换为使用物理像素表示的长度。
	inline float ConvertDipsToPixels(float dips, float dpi)
	{
		static const float dipsPerInch = 96.0f;
		return floorf(dips * dpi / dipsPerInch + 0.5f); // 舍入到最接近的整数。
	}
}

DeviceContext::DeviceContext()
	:_logger(ref new NS_CORE::Logger(DeviceContext::typeid->FullName)),
	_rotation(DXGI_MODE_ROTATION_UNSPECIFIED),
	_compositionScaleX(1.f), _compositionScaleY(1.f), _dpi(96.f)
{
	CreateDeviceIndependentResources();
	CreateDeviceResoures();
}

void DeviceContext::SetSwapChainChangedHandler(std::function<void(IDXGISwapChain*)> handler)
{
	_swapChainChangedHandler = std::move(handler);
}

void DeviceContext::UpdateDisplayMetrics(float logicalWidth, float logicalHeight, DXGI_MODE_ROTATION rotation, float compositionScaleX, float compositionScaleY, float dpi)
{
	if (_logicalWidth != logicalWidth || _logicalHeight != logicalHeight || 
		_rotation != rotation || _compositionScaleX != compositionScaleX ||
		_compositionScaleY != _compositionScaleY || _dpi != dpi)
	{
		_logicalWidth = logicalWidth;
		_logicalHeight = logicalHeight;
		_rotation = rotation;
		_compositionScaleX = compositionScaleX;
		_compositionScaleY = compositionScaleY;
		_dpi = dpi;

		CreateWindowSizeDependentResources();
	}
}

void DeviceContext::CreateDeviceIndependentResources()
{

}

void DeviceContext::CreateDeviceResoures()
{
#if defined(_DEBUG)
	// 如果项目处于调试生成阶段，请通过 SDK 层启用调试。
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
		}
	}
#endif
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory)));

	ComPtr<IDXGIAdapter1> adapter;
	GetHardwareAdapter(_dxgiFactory.Get(), &adapter);
	DumpAdapterInfo(_logger, adapter.Get());
	auto hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&_d3dDevice));
	if (FAILED(hr))
	{
		_logger->Information(L"使用 WARP 设备。");
		// 如果初始化失败，则回退到 WARP 设备。
		ComPtr<IDXGIAdapter> warpAdapter;
		ThrowIfFailed(_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
		DumpAdapterInfo(_logger, warpAdapter.Get());

		hr = D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&_d3dDevice));
	}
	ThrowIfFailed(hr);

	// 创建命令队列。
	D3D12_COMMAND_QUEUE_DESC queueDesc{};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	ThrowIfFailed(_d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&_commandQueue)));

	// 为 RTV 创建描述符堆。
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
	rtvHeapDesc.NumDescriptors = FrameCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(_d3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&_rtvDescHeap)));
	_rtvDescHandleSize = _d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// 为 DSV 创建描述符堆。
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
	rtvHeapDesc.NumDescriptors = 1;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(_d3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&_dsvDescHeap)));

	std::for_each(_commandAllocators.begin(), _commandAllocators.end(), [&](auto& allocator)
	{ _d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator)); });
	// 创建同步对象。
	ThrowIfFailed(_d3dDevice->CreateFence(CurrentFence(), D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence)));
	CurrentFence()++;
	_fenceEvent.Attach(CreateEventEx(nullptr, nullptr, FALSE, EVENT_ALL_ACCESS));
}

void DeviceContext::CreateWindowSizeDependentResources()
{
	WaitForGpu();

	// 清除特定于先前窗口大小的内容，并更新所跟踪的围栏值。
	std::for_each(_renderTargets.begin(), _renderTargets.end(), [](auto& renderTarget) {renderTarget.Reset(); });
	std::fill(_fenceValues.begin(), _fenceValues.end(), CurrentFence());

	UpdateOutputSize();
	// 交换链的宽度和高度必须基于窗口的
	// 面向本机的宽度和高度。如果窗口不在本机
	// 方向，则必须使尺寸反转。

	bool swapDimensions = _rotation == DXGI_MODE_ROTATION_ROTATE90 || _rotation == DXGI_MODE_ROTATION_ROTATE270;
	_renderTargetWidth = swapDimensions ? _outputHeight : _outputWidth;
	_renderTargetHeight = swapDimensions ? _outputWidth : _outputHeight;

	UINT backBufferWidth = lround(_renderTargetWidth);
	UINT backBufferHeight = lround(_renderTargetHeight);
	if (_swapChain)
	{
		// 如果交换链已存在，请调整其大小。
		auto hr = _swapChain->ResizeBuffers(FrameCount, backBufferWidth, backBufferHeight, _backBufferFormat, 0);

		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			// 如果出于任何原因移除了设备，将需要创建一个新的设备和交换链。
			_deviceRemoved = true;

			// 请勿继续执行此方法。会销毁并重新创建 DeviceResources。
			return;
		}
		else
			ThrowIfFailed(hr);
	}
	else
	{
		// 否则，使用与现有 Direct3D 设备相同的适配器新建一个。
		DXGI_SCALING scaling = DXGI_SCALING_STRETCH;
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};

		swapChainDesc.Width = backBufferWidth;						// 匹配窗口的大小。
		swapChainDesc.Height = backBufferHeight;
		swapChainDesc.Format = _backBufferFormat;
		swapChainDesc.Stereo = false;
		swapChainDesc.SampleDesc.Count = 1;							// 请不要使用多采样。
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = FrameCount;					    // 使用三重缓冲最大程度地减小延迟。
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	// 所有 Windows 通用应用都必须使用 _FLIP_ SwapEffects。
		swapChainDesc.Flags = 0;
		swapChainDesc.Scaling = scaling;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

		ComPtr<IDXGISwapChain1> swapChain;
		ThrowIfFailed(_dxgiFactory->CreateSwapChainForComposition(_commandQueue.Get(), &swapChainDesc, nullptr, &swapChain));
		ThrowIfFailed(swapChain.As(&_swapChain));
		OnSwapChainChanged(_swapChain.Get());
	}
	ThrowIfFailed(_swapChain->SetRotation(_rotation));

	// 创建交换链后台缓冲区的呈现目标视图。
	{
		_currentFrame = _swapChain->GetCurrentBackBufferIndex();
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescriptor(_rtvDescHeap->GetCPUDescriptorHandleForHeapStart());
		for (UINT i = 0; i < _renderTargets.size(); i++)
		{
			auto& renderTarget = _renderTargets.at(i);
			ThrowIfFailed(_swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTarget)));
			_d3dDevice->CreateRenderTargetView(renderTarget.Get(), nullptr, rtvDescriptor);
			rtvDescriptor.Offset(_rtvDescHandleSize);
		}
	}
	// 创建深度模具和视图。
	{
		D3D12_HEAP_PROPERTIES depthHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

		D3D12_RESOURCE_DESC depthResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(_depthBufferFormat, backBufferWidth, backBufferHeight);
		depthResourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		CD3DX12_CLEAR_VALUE depthOptimizedClearValue(_depthBufferFormat, 1.0f, 0);

		ThrowIfFailed(_d3dDevice->CreateCommittedResource(
			&depthHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&depthResourceDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&depthOptimizedClearValue,
			IID_PPV_ARGS(&_depthStencil)
		));

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc {};
		dsvDesc.Format = _depthBufferFormat;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

		auto handle1 = _dsvDescHeap->GetCPUDescriptorHandleForHeapStart();
		auto handle2 = _dsvDescHeap->GetGPUDescriptorHandleForHeapStart();
		_d3dDevice->CreateDepthStencilView(_depthStencil.Get(), &dsvDesc, _dsvDescHeap->GetCPUDescriptorHandleForHeapStart());
	}
	// 设置用于确定整个窗口的 3D 渲染视区。
	_screenViewport = { 0.0f, 0.0f, _renderTargetWidth, _renderTargetHeight, 0.0f, 1.0f };
}

void DeviceContext::WaitForGpu()
{
	// 在队列中安排信号命令。
	ThrowIfFailed(_commandQueue->Signal(_fence.Get(), CurrentFence()));

	// 等待跨越围栏。
	ThrowIfFailed(_fence->SetEventOnCompletion(CurrentFence(), _fenceEvent.Get()));
	WaitForSingleObjectEx(_fenceEvent.Get(), INFINITE, FALSE);

	// 对当前帧递增围栏值。
	CurrentFence()++;
}

void DeviceContext::UpdateOutputSize()
{
	// 计算必要的呈现目标大小(以像素为单位)。
	_outputWidth = ConvertDipsToPixels(_logicalWidth, _dpi);
	_outputHeight = ConvertDipsToPixels(_logicalHeight, _dpi);

	// 防止创建大小为零的 DirectX 内容。
	_outputWidth = std::max(_outputWidth, 1.f);
	_outputHeight = std::max(_outputHeight, 1.f);
}

void DeviceContext::OnSwapChainChanged(IDXGISwapChain * swapChain)
{
	if (_swapChainChangedHandler)
		_swapChainChangedHandler(swapChain);
}

void DeviceContext::CreateCommandList(UINT nodeMask, D3D12_COMMAND_LIST_TYPE type, ID3D12PipelineState * pInitialState, REFIID riid, void** ppCommandList)
{
	ThrowIfFailed(_d3dDevice->CreateCommandList(nodeMask, type, CurrentCommandAllocator, pInitialState, riid, ppCommandList));
}