//
// Red Alert Plus Engine
// Device Context
// ���ߣ�SunnyCase
// ����ʱ�䣺2016-05-25
//
#include "pch.h"
#include "DeviceContext.h"
#include "../include/d3dx12.h"
#include <sstream>

using namespace NS_ENGINE;
using namespace WRL;

namespace
{

	// �˷�����ȡ֧�� Direct3D 12 �ĵ�һ������Ӳ����������
	// ����Ҳ����������������� *ppAdapter ������Ϊ nullptr��
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
				// ��Ҫѡ�������������������������
				continue;
			}

			// ����������Ƿ�֧�� Direct3D 12������Ҫ����
			// ��Ϊʵ���豸��
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

	// ��ʹ�����豸�޹ص�����(DIP)��ʾ�ĳ���ת��Ϊʹ���������ر�ʾ�ĳ��ȡ�
	inline float ConvertDipsToPixels(float dips, float dpi)
	{
		static const float dipsPerInch = 96.0f;
		return floorf(dips * dpi / dipsPerInch + 0.5f); // ���뵽��ӽ���������
	}
}

DeviceContext::DeviceContext()
	:_logger(ref new NS_CORE::Logger(DeviceContext::typeid->FullName)),
	_rotation(DXGI_MODE_ROTATION_UNSPECIFIED),
	_compositionScaleX(1.f), _compositionScaleY(1.f), _dpi(96.f),
	_textureManager(*this)
{
	CreateDeviceIndependentResources();
	CreateDeviceResources();

	_textureManager.Initialize();
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

void DeviceContext::ExecuteCommandList(ID3D12CommandList * commandList)
{
	_commandQueue->ExecuteCommandLists(1, &commandList);
}

void DeviceContext::CreateDeviceIndependentResources()
{
	static const size_t width = 100, height = 200;
	std::stringstream ss;
	for (size_t i = 0; i < height; i++)
	{
		ss << "[";
		for (size_t j = 0; j < width; j++)
		{
			ss << "[ " << rand() % 8 << ", 0 ]";
			if (j != width - 1)
				ss << ",";
		}
		ss << "]";
		if (i != height - 1)
			ss << ",";
	}
	auto m = ss.str();
	m.data();
}

void DeviceContext::CreateDeviceResources()
{
#if defined(_DEBUG)
	// �����Ŀ���ڵ������ɽ׶Σ���ͨ�� SDK �����õ��ԡ�
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
		_logger->Information(L"ʹ�� WARP �豸��");
		// �����ʼ��ʧ�ܣ�����˵� WARP �豸��
		ComPtr<IDXGIAdapter> warpAdapter;
		ThrowIfFailed(_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
		DumpAdapterInfo(_logger, warpAdapter.Get());

		hr = D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&_d3dDevice));
	}
	ThrowIfFailed(hr);

	// ����������С�
	D3D12_COMMAND_QUEUE_DESC queueDesc{};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	ThrowIfFailed(_d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&_commandQueue)));

	// Ϊ RTV �����������ѡ�
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
	rtvHeapDesc.NumDescriptors = FrameCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(_d3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&_rtvDescHeap)));
	_rtvDescHandleSize = _d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// Ϊ DSV �����������ѡ�
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(_d3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&_dsvDescHeap)));

	std::for_each(_commandAllocators.begin(), _commandAllocators.end(), [&](auto& allocator)
	{ ThrowIfFailed(_d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator))); });
	// ����ͬ������
	ThrowIfFailed(_d3dDevice->CreateFence(CurrentFence(), D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence)));
	CurrentFence()++;
	_fenceEvent.Attach(CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS));
}

void DeviceContext::CreateWindowSizeDependentResources()
{
	WaitForGpu();

	// ����ض�����ǰ���ڴ�С�����ݣ������������ٵ�Χ��ֵ��
	std::for_each(_renderTargets.begin(), _renderTargets.end(), [](auto& renderTarget) {renderTarget.Reset(); });
	std::fill(_fenceValues.begin(), _fenceValues.end(), CurrentFence());

	UpdateOutputSize();
	// �������Ŀ�Ⱥ͸߶ȱ�����ڴ��ڵ�
	// ���򱾻��Ŀ�Ⱥ͸߶ȡ�������ڲ��ڱ���
	// ���������ʹ�ߴ練ת��

	bool swapDimensions = _rotation == DXGI_MODE_ROTATION_ROTATE90 || _rotation == DXGI_MODE_ROTATION_ROTATE270;
	_renderTargetWidth = swapDimensions ? _outputHeight : _outputWidth;
	_renderTargetHeight = swapDimensions ? _outputWidth : _outputHeight;

	UINT backBufferWidth = lround(_renderTargetWidth);
	UINT backBufferHeight = lround(_renderTargetHeight);
	if (_swapChain)
	{
		// ����������Ѵ��ڣ���������С��
		auto hr = _swapChain->ResizeBuffers(FrameCount, backBufferWidth, backBufferHeight, _backBufferFormat, 0);

		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			// ��������κ�ԭ���Ƴ����豸������Ҫ����һ���µ��豸�ͽ�������
			_deviceRemoved = true;

			// �������ִ�д˷����������ٲ����´��� DeviceResources��
			return;
		}
		else
			ThrowIfFailed(hr);
	}
	else
	{
		// ����ʹ�������� Direct3D �豸��ͬ���������½�һ����
		DXGI_SCALING scaling = DXGI_SCALING_STRETCH;
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};

		swapChainDesc.Width = backBufferWidth;						// ƥ�䴰�ڵĴ�С��
		swapChainDesc.Height = backBufferHeight;
		swapChainDesc.Format = _backBufferFormat;
		swapChainDesc.Stereo = false;
		swapChainDesc.SampleDesc.Count = 1;							// �벻Ҫʹ�ö������
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = FrameCount;					    // ʹ�����ػ������̶ȵؼ�С�ӳ١�
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;	// ���� Windows ͨ��Ӧ�ö�����ʹ�� _FLIP_ SwapEffects��
		swapChainDesc.Flags = 0;
		swapChainDesc.Scaling = scaling;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

		ComPtr<IDXGISwapChain1> swapChain;
		ThrowIfFailed(_dxgiFactory->CreateSwapChainForComposition(_commandQueue.Get(), &swapChainDesc, nullptr, &swapChain));
		ThrowIfFailed(swapChain.As(&_swapChain));
		OnSwapChainChanged(_swapChain.Get());
	}
	ThrowIfFailed(_swapChain->SetRotation(_rotation));

	// ������������̨�������ĳ���Ŀ����ͼ��
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
	// �������ģ�ߺ���ͼ��
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

		_d3dDevice->CreateDepthStencilView(_depthStencil.Get(), &dsvDesc, _dsvDescHeap->GetCPUDescriptorHandleForHeapStart());
	}
	// ��������ȷ���������ڵ� 3D ��Ⱦ������
	_screenViewport = { 0.0f, 0.0f, _renderTargetWidth, _renderTargetHeight, 0.0f, 1.0f };
}

void DeviceContext::WaitForGpu()
{
	// �ڶ����а����ź����
	ThrowIfFailed(_commandQueue->Signal(_fence.Get(), CurrentFence()));

	// �ȴ���ԽΧ����
	ThrowIfFailed(_fence->SetEventOnCompletion(CurrentFence(), _fenceEvent.Get()));
	WaitForSingleObjectEx(_fenceEvent.Get(), INFINITE, FALSE);

	// �Ե�ǰ֡����Χ��ֵ��
	CurrentFence()++;
	_resourcesWaitForUpload.clear();
}

void DeviceContext::UpdateOutputSize()
{
	// �����Ҫ�ĳ���Ŀ���С(������Ϊ��λ)��
	_outputWidth = ConvertDipsToPixels(_logicalWidth, _dpi);
	_outputHeight = ConvertDipsToPixels(_logicalHeight, _dpi);

	// ��ֹ������СΪ��� DirectX ���ݡ�
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

// ׼��������һ֡��
void DeviceContext::MoveToNextFrame()
{
	// �ڶ����а����ź����
	const auto currentFenceValue = CurrentFence();
	ThrowIfFailed(_commandQueue->Signal(_fence.Get(), currentFenceValue));

	// ���֡������
	_currentFrame = _swapChain->GetCurrentBackBufferIndex();

	// �����һ֡�Ƿ�׼����������
	if (_fence->GetCompletedValue() < CurrentFence())
	{
		ThrowIfFailed(_fence->SetEventOnCompletion(CurrentFence(), _fenceEvent.Get()));
		WaitForSingleObjectEx(_fenceEvent.Get(), INFINITE, FALSE);
	}

	// Ϊ��һ֡����Χ��ֵ��
	CurrentFence() = currentFenceValue + 1;
}

void DeviceContext::Present()
{
	// ��һ������ָʾ DXGI ������ֱֹ�� VSync����ʹӦ�ó���
	// ����һ�� VSync ǰ�������ߡ��⽫ȷ�����ǲ����˷��κ�������Ⱦ
	// �Ӳ�������Ļ����ʾ��֡��
	HRESULT hr = _swapChain->Present(1, 0);

	// ���ͨ���Ͽ����ӻ��������������Ƴ����豸�������
	// �������´��������豸��Դ��
	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
	{
		_deviceRemoved = true;
	}
	else
	{
		ThrowIfFailed(hr);

		MoveToNextFrame();
	}
}

void DeviceContext::UploadResource(IUnknown * resource)
{
	_resourcesWaitForUpload.emplace_back(resource);
}

WRL::ComPtr<ID3D12Resource> DeviceContext::CreateVertexBuffer(ID3D12GraphicsCommandList * commandList, const void * data, size_t dataSize, size_t stride, D3D12_VERTEX_BUFFER_VIEW& vertexBufferView)
{
	ComPtr<ID3D12Resource> vertexBuffer;
	// �� GPU ��Ĭ�϶��д������㻺������Դ��ʹ�����ضѽ��������ݸ��Ƶ����С�
	// �� GPU ʹ����֮ǰ�������ͷ�������Դ��
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferUpload;

	CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC vertexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(dataSize);
	ThrowIfFailed(_d3dDevice->CreateCommittedResource(
		&defaultHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&vertexBufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&vertexBuffer)));

	CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
	ThrowIfFailed(_d3dDevice->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&vertexBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertexBufferUpload)));
	// �����㻺�������ص� GPU��
	{
		D3D12_SUBRESOURCE_DATA vertexData = {};
		vertexData.pData = data;
		vertexData.RowPitch = dataSize;
		vertexData.SlicePitch = vertexData.RowPitch;

		UpdateSubresources(commandList, vertexBuffer.Get(), vertexBufferUpload.Get(), 0, 0, 1, &vertexData);

		CD3DX12_RESOURCE_BARRIER vertexBufferResourceBarrier =
			CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		commandList->ResourceBarrier(1, &vertexBufferResourceBarrier);
	}
	UploadResource(vertexBufferUpload.Get());
	// ��������/������������ͼ��
	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.StrideInBytes = stride;
	vertexBufferView.SizeInBytes = dataSize;
	return vertexBuffer;
}
