#include "stdafx.h"
#include "Converter.h"

using namespace WRL;

namespace
{
	void DrawFrame(uint32_t* target, uint32_t left, uint32_t top, uint32_t pitch, const Palette& pal, const ShpFrame& frame)
	{
		if (frame.RawData.empty())return;
		auto data = std::make_unique<uint32_t[]>(frame.CroppedWidth * frame.CroppedHeight);
		auto ptr = data.get();
		for (auto&& pixel : frame.RawData)
		{
			auto palColor = pal.GetColors()[pixel];
			uint32_t color = (palColor.r << 16) | (palColor.g << 8) | palColor.b;
			if (pixel == 0)
				color = 0;
			else if (pixel >= 16 && pixel <= 31)
				color = color;
			else
				color = color | 0xFF000000;
			*ptr++ = color;
		}
		auto xStart = target + (top + frame.OffsetY) * pitch + left + frame.OffsetX;
		for (size_t y = 0; y < frame.CroppedHeight; y++)
		{
			auto cnt = xStart;
			for (size_t x = 0; x < frame.CroppedWidth; x++)
				*cnt++ = data[y * frame.CroppedWidth + x];
			xStart += pitch;
		}
	}

	uint32_t FitTo4(uint32_t value)
	{
		auto rest = value % 4;
		return rest ? value + 4 - rest : value;
	}
}

Converter::Converter(const Palette & palette, const Shp & shp)
	:_palette(palette), _shp(shp)
{
	ThrowIfFailed(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&_wicFactory)));
}

void Converter::Save(const std::wstring & fileName)
{
	auto columns = std::max(6u, (uint32_t)std::sqrt(_shp.GetFrameCount()));
	auto rows = (uint32_t)std::ceil((double)_shp.GetFrameCount() / columns);
	const auto width = FitTo4(columns * _shp.GetFrameWidth());
	const auto height = FitTo4(rows * _shp.GetFrameHeight());

	ComPtr<IWICBitmap> targetBitmap;
	ThrowIfFailed(_wicFactory->CreateBitmap(width, height, GUID_WICPixelFormat32bppBGRA, WICBitmapCacheOnDemand, &targetBitmap));
	
	WICRect rect{ 0, 0, width, height };
	{
		ComPtr<IWICBitmapLock> locker;
		ThrowIfFailed(targetBitmap->Lock(&rect, WICBitmapLockWrite, &locker)); 
		
		UINT cbBufferSize = 0;
		UINT cbStride = 0;
		BYTE *pv = NULL;
		ThrowIfFailed(locker->GetDataPointer(&cbBufferSize, &pv));
		ThrowIfFailed(locker->GetStride(&cbStride));

		uint32_t x = 0, y = 0;
		for (auto&& frame : _shp.GetFrames())
		{
			DrawFrame(reinterpret_cast<uint32_t*>(pv), x * _shp.GetFrameWidth(), y * _shp.GetFrameHeight(), cbStride / 4, _palette, frame);
			if (++x >= columns)
			{
				y++;
				x = 0;
			}
		}
	}

	ComPtr<IWICDdsEncoder> ddsEncoder;
	ComPtr<IWICBitmapEncoder> bitmapEncoder;
	ThrowIfFailed(_wicFactory->CreateEncoder(GUID_ContainerFormatDds, NULL, &bitmapEncoder));
	ThrowIfFailed(bitmapEncoder.As(&ddsEncoder));

	ComPtr<IWICStream> outputStream;
	ThrowIfFailed(_wicFactory->CreateStream(&outputStream));
	ThrowIfFailed(outputStream->InitializeFromFilename(fileName.c_str(), GENERIC_WRITE));
	ThrowIfFailed(bitmapEncoder->Initialize(outputStream.Get(), WICBitmapEncoderNoCache));

	WICDdsParameters parameters{};
	parameters.Width = width;
	parameters.Height = height;
	parameters.ArraySize = 1;
	parameters.AlphaMode = WICDdsAlphaModeStraight;
	parameters.Dimension = WICDdsTexture2D;
	parameters.DxgiFormat = DXGI_FORMAT_BC2_UNORM;
	parameters.MipLevels = 1;
	parameters.Depth = 1;
	ThrowIfFailed(ddsEncoder->SetParameters(&parameters));

	ComPtr<IWICBitmapFrameEncode> frameEncoder;
	ThrowIfFailed(bitmapEncoder->CreateNewFrame(&frameEncoder, nullptr));
	ThrowIfFailed(frameEncoder->Initialize(nullptr));
	ThrowIfFailed(frameEncoder->WriteSource(targetBitmap.Get(), nullptr));
	ThrowIfFailed(frameEncoder->Commit());
	ThrowIfFailed(bitmapEncoder->Commit());
}