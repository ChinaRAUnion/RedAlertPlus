//
// Red Alert Plus
// SwapChain Provider
// ���ߣ�SunnyCase
// ����ʱ�䣺2016-05-24
//
#include "pch.h"
#include "SwapChainProvider.h"
#include <sstream>

using namespace NS_RAP;
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
}

SwapChainProvider::SwapChainProvider()
	:_logger(ref new NS_CORE::Logger(SwapChainProvider::typeid->FullName))
{
	CreateDeviceIndependentResources();
	CreateDeviceResoures();
}

void SwapChainProvider::SetSwapChainPanel(Windows::UI::Xaml::Controls::SwapChainPanel ^ panel)
{
	_panel = panel;

}

void SwapChainProvider::CreateDeviceIndependentResources()
{

}

void SwapChainProvider::CreateDeviceResoures()
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
	ComPtr<IDXGIFactory4> dxgiFactory;
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));

	ComPtr<IDXGIAdapter1> adapter;
	GetHardwareAdapter(dxgiFactory.Get(), &adapter);
	DumpAdapterInfo(_logger, adapter.Get());
	auto hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&_d3dDevice));
	if (FAILED(hr))
	{
		_logger->Information(L"ʹ�� WARP �豸��");
		// �����ʼ��ʧ�ܣ�����˵� WARP �豸��
		ComPtr<IDXGIAdapter> warpAdapter;
		ThrowIfFailed(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
		DumpAdapterInfo(_logger, warpAdapter.Get());

		hr = D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&_d3dDevice));
	}
	ThrowIfFailed(hr);

	// ����������С�
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	ThrowIfFailed(_d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&_commandQueue)));
}