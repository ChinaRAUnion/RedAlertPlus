//
// Red Alert Plus Engine
// Engine
// 作者：SunnyCase
// 创建时间：2016-05-25
//
#pragma once
#include "common.h"
#include <ppltasks.h>
#include <dxgi1_4.h>
#include <unordered_map>

DEFINE_NS_RAP

namespace Api
{
#import "../../bin/Api/Debug/ChinaRAUnion.RedAlertPlus.Api.tlb" no_smart_pointers  no_namespace
}

END_NS_RAP

DEFINE_NS_ENGINE

struct TileSetPackageContent
{
	std::wstring TileSet;
	std::vector<byte> Image;
	std::vector<byte> ExtraImage;
};

struct BlobWithContentType
{
	std::vector<byte> Blob;
	std::wstring ContentType;
};

struct MapGenerateOptions
{
	uint32_t width;
	uint32_t height;
	std::wstring tileSetName;
};

interface DECLSPEC_UUID("1B572D0E-8D92-4BE8-B71C-CD633D97134F") IResourceResovler : public IUnknown
{
	virtual concurrency::task<std::vector<byte>> ResovleShader(const std::wstring& name) = 0;
	virtual concurrency::task<TileSetPackageContent> ResolveTileSetPackageFile(const std::wstring& name) = 0;
	virtual concurrency::task<BlobWithContentType> ResolveTexture(const std::wstring& name) = 0;
	virtual concurrency::task<std::wstring> ResolveMap(const std::wstring& name) = 0;
};

interface DECLSPEC_UUID("89D1AC22-2CBC-46A2-A0DC-E6A09CDC7A6C") IRulesResolver : public IUnknown
{
	virtual const std::unordered_map<std::wstring, Api::IInfantry*>& get_Infantry() = 0;
};

interface DECLSPEC_UUID("DDDF5DA6-E458-4894-ACFC-5045A30676F7") IEngine : public IUnknown
{
	virtual void SetSwapChainChangedHandler(std::function<void(IDXGISwapChain*)> handler) = 0;
	virtual void UpdateDisplayMetrics(float logicalWidth, float logicalHeight, DXGI_MODE_ROTATION rotation, float compositionScaleX, float compositionScaleY, float dpi) = 0;

	virtual void UseMap(const std::wstring& mapData) = 0;
	virtual void GenerateMap(const MapGenerateOptions& options) = 0;
	virtual concurrency::task<void> InitializeAsync() = 0;
	virtual void Render() = 0;

	virtual void SetMapScrollSpeed(float x, float y) = 0;
};


END_NS_ENGINE

extern "C"
{
	CRAU_ENGINE_API HRESULT STDMETHODCALLTYPE CreateEngine(NS_ENGINE::IEngine** engine, NS_ENGINE::IResourceResovler* resourceResolver);
}