//
// Red Alert Plus Engine
// TileSetGraphicsProvider 实现
// 作者：SunnyCase
// 创建时间：2016-06-12
//
#include "pch.h"
#include "TileSetGraphicsProviderImpl.h"
#include <d3d11.h>
#include <sstream>

using namespace concurrency;

HRESULT STDMETHODCALLTYPE CreateTileSetGraphicsProvider(NS_ENGINE::ITileSetGraphicsProvider** provider, NS_ENGINE::IResourceResovler* resourceResolver)
{
	try
	{
		ARGUMENT_NOTNULL_HR(provider);
		*provider = WRL::Make<NS_ENGINE::TileSetGraphicsProvider>(resourceResolver).Detach();
		return S_OK;
	}
	CATCH_ALL();
}

using namespace NS_ENGINE;
using namespace WRL;

namespace
{
	static const D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};

	// 此方法获取支持 Direct3D 11 的第一个可用硬件适配器。
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

			// 检查适配器是否支持 Direct3D 11，但不要创建
			// 仍为实际设备。
			if (SUCCEEDED(D3D11CreateDevice(adapter.Get(), D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_BGRA_SUPPORT, featureLevels,
				_countof(featureLevels), D3D11_SDK_VERSION, nullptr, nullptr, nullptr)))
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
TileSetGraphicsProvider::TileSetGraphicsProvider(IResourceResovler * resourceResolver)
	:_resourceResolver(resourceResolver), _logger(ref new Tomato::Core::Logger(TileSetGraphicsProvider::typeid->FullName))
{
	ThrowIfFailed(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&_wicFactory)));
	ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, _d2dFactory.GetAddressOf()));
	CreateDeviceResources();
}

concurrency::task<void> TileSetGraphicsProvider::InitializeAsync(const std::wstring & tileSetName)
{
	auto res = co_await _resourceResolver->ResolveTileSetPackageFile(tileSetName);
	DecodeImage(res.Image, &_image);
	DecodeImage(res.ExtraImage, &_extraImage);

	_imageSize = _image->GetPixelSize();
	_extImageSize = _extraImage->GetPixelSize();
	_tileSetInfo.~TileSetInfo();
	new (&_tileSetInfo) TileSetInfo(res.TileSet, _imageSize.width, _imageSize.height, _extImageSize.width, _extImageSize.height);
}

void TileSetGraphicsProvider::CreatePickAnyUnit(uint32_t id, IDXGISurface** surface)
{
	if (!surface) ThrowIfFailed(E_POINTER);
	auto pau = _tileSetInfo.FindPickAnyUnit(id);
	if (pau.Tiles.empty())
		*surface = nullptr;
	else
	{
		const auto& tile = _tileSetInfo.FindTile(pau.Tiles.front());
		Tomato::BoundingRect bounding(0, _tileSetInfo.TileHeight, _tileSetInfo.TileWidth, 0);
		auto x = tile.LeftTopUV.x * _imageSize.width;
		auto y = tile.LeftTopUV.y * _imageSize.height;

		ComPtr<IDXGISurface> dxgiSurface;
		{
			ComPtr<ID3D11Texture2D> texture;
			D3D11_TEXTURE2D_DESC texDesc;
			texDesc.ArraySize = 1;
			texDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
			texDesc.CPUAccessFlags = 0;
			texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			texDesc.Height = UINT(bounding.Height);
			texDesc.Width = UINT(bounding.Width);
			texDesc.MipLevels = 1;
			texDesc.MiscFlags = 0;
			texDesc.SampleDesc.Count = 1;
			texDesc.SampleDesc.Quality = 0;
			texDesc.Usage = D3D11_USAGE_DEFAULT;
			ThrowIfFailed(_d3dDevice->CreateTexture2D(&texDesc, nullptr, &texture));
			ThrowIfFailed(texture.As(&dxgiSurface));
		}
		ComPtr<ID2D1RenderTarget> target;
		ThrowIfFailed(_d2dFactory->CreateDxgiSurfaceRenderTarget(dxgiSurface.Get(),
			D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)), &target));
		ComPtr<ID2D1Bitmap> image;
		ThrowIfFailed(target->CreateSharedBitmap(__uuidof(ID2D1Bitmap), _image.Get(), nullptr, &image));
		
		target->BeginDraw();
		target->DrawBitmap(image.Get(), nullptr, 1.f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &D2D1::RectF(x, y, x + _tileSetInfo.TileWidth, y + _tileSetInfo.TileHeight));
		ThrowIfFailed(target->EndDraw());
		ThrowIfFailed(dxgiSurface.CopyTo(surface));
	}
}

void TileSetGraphicsProvider::DecodeImage(const std::vector<byte>& data, ID2D1Bitmap** bitmap)
{
	ComPtr<IWICStream> stream;
	ThrowIfFailed(_wicFactory->CreateStream(&stream));
	ThrowIfFailed(stream->InitializeFromMemory(const_cast<byte*>(data.data()), data.size()));

	ComPtr<IWICBitmapDecoder> decoder;
	ThrowIfFailed(_wicFactory->CreateDecoderFromStream(stream.Get(), nullptr, WICDecodeMetadataCacheOnDemand, &decoder));
	ComPtr<IWICBitmapFrameDecode> frame;
	ThrowIfFailed(decoder->GetFrame(0, &frame));

	WICPixelFormatGUID pixelFormat;
	ThrowIfFailed(frame->GetPixelFormat(&pixelFormat));

	if (pixelFormat != GUID_WICPixelFormat32bppPBGRA)
	{
		ComPtr<IWICFormatConverter> formatConverter;
		ThrowIfFailed(_wicFactory->CreateFormatConverter(&formatConverter));
		ThrowIfFailed(formatConverter->Initialize(frame.Get(), GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom));
		ThrowIfFailed(_d2dDeviceContext->CreateBitmapFromWicBitmap(formatConverter.Get(), bitmap));
	}
	else
		ThrowIfFailed(_d2dDeviceContext->CreateBitmapFromWicBitmap(frame.Get(), bitmap));
}

void TileSetGraphicsProvider::CreateDeviceResources()
{
	ComPtr<IDXGIFactory4> dxgiFactory;
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));

	ComPtr<IDXGIAdapter1> adapter;
	GetHardwareAdapter(dxgiFactory.Get(), &adapter);
	DumpAdapterInfo(_logger, adapter.Get());

	auto flag = D3D11_CREATE_DEVICE_BGRA_SUPPORT
#if _DEBUG
		| D3D11_CREATE_DEVICE_DEBUG
#endif
		;
	auto hr = D3D11CreateDevice(adapter.Get(), D3D_DRIVER_TYPE_HARDWARE, NULL, flag, featureLevels,
		_countof(featureLevels), D3D11_SDK_VERSION, &_d3dDevice, nullptr, nullptr);
	if (FAILED(hr))
	{
		_logger->Information(L"使用 WARP 设备。");
		// 如果初始化失败，则回退到 WARP 设备。
		ComPtr<IDXGIAdapter> warpAdapter;
		ThrowIfFailed(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
		DumpAdapterInfo(_logger, warpAdapter.Get());

		hr = D3D11CreateDevice(adapter.Get(), D3D_DRIVER_TYPE_WARP, NULL, flag, featureLevels,
			_countof(featureLevels), D3D11_SDK_VERSION, &_d3dDevice, nullptr, nullptr);
	}
	ThrowIfFailed(hr);
	ComPtr<IDXGIDevice> dxgiDevice;
	ThrowIfFailed(_d3dDevice.As(&dxgiDevice));
	ThrowIfFailed(_d2dFactory->CreateDevice(dxgiDevice.Get(), &_d2dDevice));
	ThrowIfFailed(_d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &_d2dDeviceContext));
}