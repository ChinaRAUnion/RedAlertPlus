//
// Red Alert Plus Engine
// Texture Manager
// 作者：SunnyCase
// 创建时间：2016-05-30
//
#include "pch.h"
#include "TextureManager.h"
#include "../DeviceContext.h"
#include <unordered_map>

using namespace NS_ENGINE;
using namespace WRL;

namespace std
{
	template<>
	struct hash<GUID>
	{
		size_t operator()(const GUID& target) const
		{
			return target.Data1 ^ ((size_t)target.Data2 << 16 | (size_t)(target.Data3)) ^ ((size_t)target.Data4[2] << 24 | (size_t)target.Data4[7]);
		}
	};
}

namespace
{
	static std::unordered_map<WICPixelFormatGUID, DXGI_FORMAT> g_WICFormats =
	{
		{ GUID_WICPixelFormat128bppRGBAFloat,       DXGI_FORMAT_R32G32B32A32_FLOAT },

		{ GUID_WICPixelFormat64bppRGBAHalf,         DXGI_FORMAT_R16G16B16A16_FLOAT },
		{ GUID_WICPixelFormat64bppRGBA,             DXGI_FORMAT_R16G16B16A16_UNORM },

		{ GUID_WICPixelFormat32bppRGBA,             DXGI_FORMAT_R8G8B8A8_UNORM },
		{ GUID_WICPixelFormat32bppBGRA,             DXGI_FORMAT_B8G8R8A8_UNORM }, // DXGI 1.1
		{ GUID_WICPixelFormat32bppBGR,              DXGI_FORMAT_B8G8R8X8_UNORM }, // DXGI 1.1

		{ GUID_WICPixelFormat32bppRGBA1010102XR,    DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM }, // DXGI 1.1
		{ GUID_WICPixelFormat32bppRGBA1010102,      DXGI_FORMAT_R10G10B10A2_UNORM },

		{ GUID_WICPixelFormat16bppBGRA5551,         DXGI_FORMAT_B5G5R5A1_UNORM },
		{ GUID_WICPixelFormat16bppBGR565,           DXGI_FORMAT_B5G6R5_UNORM },

		{ GUID_WICPixelFormat32bppGrayFloat,        DXGI_FORMAT_R32_FLOAT },
		{ GUID_WICPixelFormat16bppGrayHalf,         DXGI_FORMAT_R16_FLOAT },
		{ GUID_WICPixelFormat16bppGray,             DXGI_FORMAT_R16_UNORM },
		{ GUID_WICPixelFormat8bppGray,              DXGI_FORMAT_R8_UNORM },

		{ GUID_WICPixelFormat8bppAlpha,             DXGI_FORMAT_A8_UNORM },
		{ GUID_WICPixelFormat96bppRGBFloat,			DXGI_FORMAT_R32G32B32_FLOAT }
	};

	static DXGI_FORMAT WICToDXGI(const WICPixelFormatGUID& guid)
	{
		auto it = g_WICFormats.find(guid);

		return it == g_WICFormats.end() ? DXGI_FORMAT_UNKNOWN : it->second;
	}
}

TextureManager::TextureManager(DeviceContext & deviceContext)
	:_deviceContext(deviceContext)
{
	ThrowIfFailed(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&_wicFactory)));
}

void TextureManager::Initialize()
{
	auto d3dDevice = _deviceContext.D3DDevice;

	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc{};
	srvHeapDesc.NumDescriptors = HandlesPerHeap;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&_srvDescHeap)));
	_srvDescHandleSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

Texture TextureManager::GetTexutre2DInfo(const byte * data, size_t size, const std::wstring & contentType)
{
	if (contentType == L"image/dds")
		ThrowAlways(L"DDS is not supported.");

	ComPtr<IWICStream> stream;
	ThrowIfFailed(_wicFactory->CreateStream(&stream));
	ThrowIfFailed(stream->InitializeFromMemory(const_cast<byte*>(data), size));

	ComPtr<IWICBitmapDecoder> decoder;
	ThrowIfFailed(_wicFactory->CreateDecoderFromStream(stream.Get(), nullptr, WICDecodeMetadataCacheOnDemand, &decoder));
	ComPtr<IWICBitmapFrameDecode> frame;
	ThrowIfFailed(decoder->GetFrame(0, &frame));

	UINT width, height;
	ThrowIfFailed(frame->GetSize(&width, &height));
	WICPixelFormatGUID wicFormat;
	ThrowIfFailed(frame->GetPixelFormat(&wicFormat));
	auto dxgiFormat = WICToDXGI(wicFormat);

	return Texture{ nullptr, {}, {}, width, height };
}

Texture TextureManager::CreateTexture2D(const byte * data, size_t size, const std::wstring & contentType)
{
	if (contentType == L"image/dds")
		ThrowAlways(L"DDS is not supported.");

	ComPtr<IWICStream> stream;
	ThrowIfFailed(_wicFactory->CreateStream(&stream));
	ThrowIfFailed(stream->InitializeFromMemory(const_cast<byte*>(data), size));

	ComPtr<IWICBitmapDecoder> decoder;
	ThrowIfFailed(_wicFactory->CreateDecoderFromStream(stream.Get(), nullptr, WICDecodeMetadataCacheOnDemand, &decoder));
	ComPtr<IWICBitmapFrameDecode> frame;
	ThrowIfFailed(decoder->GetFrame(0, &frame));

	UINT width, height;
	ThrowIfFailed(frame->GetSize(&width, &height));
	WICPixelFormatGUID wicFormat;
	ThrowIfFailed(frame->GetPixelFormat(&wicFormat));
	auto dxgiFormat = WICToDXGI(wicFormat);

	auto d3dDevice = _deviceContext.D3DDevice;
	auto textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(dxgiFormat, width, height);

	Microsoft::WRL::ComPtr<ID3D12Resource> texture, textureUpload;

	CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
	ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&defaultHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&texture)));

	DataToUpload toUpload;
	// 将纹理上载到 GPU。
	{
		auto bpp = BitsPerPixel(wicFormat);
		size_t rowPitch = (width * bpp + 7) / 8;
		size_t imageSize = rowPitch * height;
		toUpload.data = std::make_unique<byte[]>(imageSize);
		ThrowIfFailed(frame->CopyPixels(nullptr, rowPitch, imageSize, toUpload.data.get()));

		auto& textureData = toUpload.resData;
		textureData.pData = toUpload.data.get();
		textureData.RowPitch = rowPitch;
		textureData.SlicePitch = imageSize;

		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(texture.Get(), 0, 1);

		CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
		ThrowIfFailed(d3dDevice->CreateCommittedResource(
			&uploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&toUpload.resForUpload)));
	}

	auto handle = AllocateHandle();
	// Describe and create a SRV for the texture.
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	d3dDevice->CreateShaderResourceView(texture.Get(), &srvDesc, handle.first);

	Texture result(texture.Get(), handle.first, handle.second, width, height);
	result.dataForUpload = std::make_unique<DataToUpload>(std::move(toUpload));
	return result;
}

void TextureManager::Upload(Texture & texture, ID3D12GraphicsCommandList * commandList, std::vector<WRL::ComPtr<IUnknown>>& resourcesWaitForUpload)
{
	ThrowIfNot(texture.dataForUpload, L"Texture is already uploaded.");
	UpdateSubresources(commandList, texture.Resource, texture.dataForUpload->resForUpload.Get(), 0, 0, 1, &texture.dataForUpload->resData);

	CD3DX12_RESOURCE_BARRIER textureResourceBarrier =
		CD3DX12_RESOURCE_BARRIER::Transition(texture.Resource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	commandList->ResourceBarrier(1, &textureResourceBarrier);
	resourcesWaitForUpload.emplace_back(texture.dataForUpload->resForUpload);
	texture.dataForUpload = nullptr;
}

size_t TextureManager::BitsPerPixel(WICPixelFormatGUID & guid)
{
	ComPtr<IWICComponentInfo> cinfo;
	ThrowIfFailed(_wicFactory->CreateComponentInfo(guid, &cinfo));

	WICComponentType type;
	ThrowIfFailed(cinfo->GetComponentType(&type));
	ThrowIfNot(type == WICPixelFormat, L"Invalid WIC component type.");

	ComPtr<IWICPixelFormatInfo> pfinfo;
	ThrowIfFailed(cinfo.As(&pfinfo));

	UINT bpp;
	ThrowIfFailed(pfinfo->GetBitsPerPixel(&bpp));
	return bpp;
}

std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> TextureManager::AllocateHandle()
{
	auto offset = _nextAvailOffset++;
	return std::make_pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE>(
		CD3DX12_CPU_DESCRIPTOR_HANDLE(_srvDescHeap->GetCPUDescriptorHandleForHeapStart(), offset, _srvDescHandleSize),
		CD3DX12_GPU_DESCRIPTOR_HANDLE(_srvDescHeap->GetGPUDescriptorHandleForHeapStart(), offset, _srvDescHandleSize));
}
