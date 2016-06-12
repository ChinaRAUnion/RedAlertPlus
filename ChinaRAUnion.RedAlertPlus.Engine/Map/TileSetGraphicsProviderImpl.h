//
// Red Alert Plus Engine
// TileSetGraphicsProvider 实现
// 作者：SunnyCase
// 创建时间：2016-06-12
//
#pragma once
#include "../../include/engine/TileSetGraphicsProvider.h"
#include "TileSetInfo.h"
#include <wincodec.h>
#include <d2d1_3.h>

DEFINE_NS_ENGINE

class TileSetGraphicsProvider : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, ITileSetGraphicsProvider>
{
public:
	TileSetGraphicsProvider(IResourceResovler* resourceResolver);

	virtual concurrency::task<void> InitializeAsync(const std::wstring& tileSetName);
	void CreatePickAnyUnit(uint32_t id, IDXGISurface** surface);
private:
	void DecodeImage(const std::vector<byte>& data, ID2D1Bitmap** bitmap);
	void CreateDeviceResources();
private:
	Tomato::Core::Logger^ _logger;
	WRL::ComPtr<IResourceResovler> _resourceResolver;
	TileSetInfo _tileSetInfo;
	WRL::ComPtr<IWICImagingFactory> _wicFactory;
	WRL::ComPtr<ID2D1Bitmap> _image, _extraImage;
	D2D1_SIZE_U _imageSize, _extImageSize;
	WRL::ComPtr<ID2D1Factory3> _d2dFactory;
	WRL::ComPtr<ID2D1Device1> _d2dDevice;
	WRL::ComPtr<ID2D1DeviceContext1> _d2dDeviceContext;
	WRL::ComPtr<ID3D11Device> _d3dDevice;
};

END_NS_ENGINE