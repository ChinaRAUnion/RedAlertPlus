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

interface DECLSPEC_UUID("1B572D0E-8D92-4BE8-B71C-CD633D97134F") IResourceResovler : public IUnknown
{
	virtual concurrency::task<std::vector<byte>> ResovleShader(const std::wstring& name) = 0;
	virtual concurrency::task<TileSetPackageContent> ResolveTileSetPackageFile(const std::wstring& name) = 0;
	virtual concurrency::task<BlobWithContentType> ResolveTexture(const std::wstring& name) = 0;
};

interface DECLSPEC_UUID("DDDF5DA6-E458-4894-ACFC-5045A30676F7") IEngine : public IUnknown
{
	virtual void SetSwapChainChangedHandler(std::function<void(IDXGISwapChain*)> handler) = 0;
	virtual void UpdateDisplayMetrics(float logicalWidth, float logicalHeight, DXGI_MODE_ROTATION rotation, float compositionScaleX, float compositionScaleY, float dpi) = 0;
	virtual concurrency::task<void> InitializeAsync() = 0;
};


END_NS_ENGINE

extern "C"
{
	CRAU_ENGINE_API HRESULT STDMETHODCALLTYPE CreateEngine(NS_ENGINE::IEngine** engine, NS_ENGINE::IResourceResovler* resourceResolver);
}