#include "stdafx.h"
#include "Converter.h"
#include <unordered_map>
#include <algorithm>
#include "../../thirdparty/rapidjson/document.h"
#include "../../thirdparty/rapidjson/stringbuffer.h"
#include "../../thirdparty/rapidjson/prettywriter.h"
#include <sstream>
#include <fstream>
#include "minizip\minizip_raii.h"

using namespace WRL;
using namespace rapidjson;

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
		auto xStart = target + top * pitch + left;
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

	std::wstring GenTempFileName()
	{
		wchar_t tmpPath[MAX_PATH];
		GetTempPath(_countof(tmpPath), tmpPath);
		wchar_t tmpName[MAX_PATH];
		GetTempFileName(tmpPath, L"shp2dds", 0, tmpName);
		return tmpName;
	}
	std::string ws2s(const std::wstring& str, UINT codePage)
	{
		auto len = WideCharToMultiByte(codePage, 0, str.data(), str.size(), nullptr, 0, nullptr, nullptr);
		std::string s(len, 0);
		WideCharToMultiByte(codePage, 0, str.data(), str.size(), &s[0], len, nullptr, nullptr);

		return s;
	}
}

Converter::Converter(const Palette & palette, const Shp & shp)
	:_palette(palette), _shp(shp)
{
	ThrowIfFailed(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&_wicFactory)));
}

void Converter::Save(const std::wstring & taniFileName)
{
	auto ddsFileName = GenTempFileName();
	StringBuffer sb;
	{
		// width group
		std::unordered_map<uint32_t, std::vector<size_t>> frameGroups;
		{
			size_t id = 0;
			for (auto&& frame : _shp.GetFrames())
				frameGroups[frame.CroppedWidth].emplace_back(id++);
		}
		uint32_t width = 0;
		{
			for (auto&& group : frameGroups)
				width += group.first;
		}
		width = FitTo4(width);
		uint32_t height = 0;
		{
			for (auto&& group : frameGroups)
			{
				uint32_t groupHeight = 0;
				for (auto&& frame : group.second)
					groupHeight += _shp.GetFrames()[frame].CroppedHeight;
				height = std::max(height, groupHeight);
			}
		}
		height = FitTo4(height);

		ComPtr<IWICBitmap> targetBitmap;
		ThrowIfFailed(_wicFactory->CreateBitmap(width, height, GUID_WICPixelFormat32bppBGRA, WICBitmapCacheOnDemand, &targetBitmap));

		MemoryPoolAllocator<> allocator;
		auto jFrames = Value(Type::kArrayType).GetArray();
		WICRect rect{ 0, 0, width, height };
		{
			ComPtr<IWICBitmapLock> locker;
			ThrowIfFailed(targetBitmap->Lock(&rect, WICBitmapLockWrite, &locker));

			UINT cbBufferSize = 0;
			UINT cbStride = 0;
			BYTE *pv = NULL;
			ThrowIfFailed(locker->GetDataPointer(&cbBufferSize, &pv));
			ThrowIfFailed(locker->GetStride(&cbStride));

			uint32_t x = 0;
			for (auto&& group : frameGroups)
			{
				uint32_t y = 0;
				for (auto&& frameId : group.second)
				{
					auto&& frame = _shp.GetFrames()[frameId];
					DrawFrame(reinterpret_cast<uint32_t*>(pv), x, y, cbStride / 4, _palette, frame);

					Value jFrame(Type::kObjectType);
					{
						Value jCoord(kObjectType);
						jCoord.AddMember("x", x, allocator);
						jCoord.AddMember("y", y, allocator);
						jFrame.AddMember("coord", jCoord, allocator);
					}
					{
						Value jOffset(kObjectType);
						jOffset.AddMember("x", frame.OffsetX, allocator);
						jOffset.AddMember("y", frame.OffsetY, allocator);
						jFrame.AddMember("offset", jOffset, allocator);
					}
					{
						Value jSize(kObjectType);
						jSize.AddMember("width", frame.CroppedWidth, allocator);
						jSize.AddMember("height", frame.CroppedHeight, allocator);
						jFrame.AddMember("size", jSize, allocator);
					}
					{
						auto color = frame.RadarColor;
						jFrame.AddMember("radarColor", uint32_t((color.a << 24) | (color.r << 16) | (color.g << 8) | color.b), allocator);
					}
					jFrames.PushBack(jFrame, allocator);
					y += frame.CroppedHeight;
				}
				x += group.first;
			}
		}
		Document doc(Type::kObjectType);
		{
			Value jSize(Type::kObjectType);
			jSize.AddMember("width", _shp.GetFrameWidth(), allocator);
			jSize.AddMember("height", _shp.GetFrameHeight(), allocator);
			doc.AddMember("size", jSize, allocator);
		}
		doc.AddMember("frames", jFrames, allocator);
		PrettyWriter<StringBuffer> writer(sb);
		doc.Accept(writer);

		ComPtr<IWICDdsEncoder> ddsEncoder;
		ComPtr<IWICBitmapEncoder> bitmapEncoder;
		ThrowIfFailed(_wicFactory->CreateEncoder(GUID_ContainerFormatDds, NULL, &bitmapEncoder));
		ThrowIfFailed(bitmapEncoder.As(&ddsEncoder));

		ComPtr<IWICStream> outputStream;
		ThrowIfFailed(_wicFactory->CreateStream(&outputStream));
		
		ThrowIfFailed(outputStream->InitializeFromFilename(ddsFileName.c_str(), GENERIC_WRITE));
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

	zipFile_raii zipFile(zipOpen(ws2s(taniFileName, CP_UTF8).c_str(), APPEND_STATUS_CREATE));
	{
		zip_fileinfo zi{};
		ThrowIfNot(zipOpenNewFileInZip(zipFile.get(), "ani.dds", &zi, nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, 3) == ZIP_OK, L"Error in create new archive.");
		FILE* fin;
		_wfopen_s(&fin, ddsFileName.c_str(), L"rb");
		int err = ZIP_OK;
		int size_read;
		byte buf[40960];
		do
		{
			size_read = (int)fread(buf, 1, sizeof(buf), fin);
			if ((size_read < sizeof(buf)) && (feof(fin) == 0))
			{
				err = ZIP_ERRNO;
			}

			if (size_read > 0)
			{
				err = zipWriteInFileInZip(zipFile.get(), buf, size_read);
			}
		} while ((err == ZIP_OK) && (size_read > 0));
		fclose(fin);
		ThrowIfNot(zipCloseFileInZip(zipFile.get()) == ZIP_OK, L"Error in close ani.dds archive.");
	}

	{
		zip_fileinfo zi{};
		ThrowIfNot(zipOpenNewFileInZip(zipFile.get(), "ani.json", &zi, nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, 3) == ZIP_OK, L"Error in create new archive.");
		ThrowIfNot(zipWriteInFileInZip(zipFile.get(), sb.GetString(), sb.GetSize()) == ZIP_OK, L"Error in write ani.json archive.");
		ThrowIfNot(zipCloseFileInZip(zipFile.get()) == ZIP_OK, L"Error in close ani.json archive.");
	}
}