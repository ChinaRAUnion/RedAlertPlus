//
// Red Alert Plus
// ResourceResolver
// 作者：SunnyCase
// 创建时间：2016-06-12
//
#pragma once
#include "../include/common.h"
#include "../include/engine/Engine.h"
#include <sstream>

DEFINE_NS_RAP

class ResourceResolver : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, NS_ENGINE::IResourceResovler>
{
public:
	ResourceResolver(IGameEngineResourceResolver^ resourceResolver)
		: _resourceResovler(resourceResolver)
	{

	}

	virtual Concurrency::task<std::vector<byte>> ResovleShader(const std::wstring &name)
	{
		auto res = _resourceResovler->ResolveShader(Platform::StringReference(name.c_str(), name.length()));
		return ReadStream(res);
	}

	virtual concurrency::task<NS_ENGINE::TileSetPackageContent> ResolveTileSetPackageFile(const std::wstring& name)
	{
		auto res = _resourceResovler->ResolveTileSetPackageFile(Platform::StringReference(name.c_str(), name.length()));
		auto tileSet = co_await ReadString(res->TileSet);
		auto image = co_await ReadStream(res->Image);
		auto extraImage = co_await ReadStream(res->ExtraImage);
		return{ tileSet, image,extraImage };
	}

	virtual concurrency::task<NS_ENGINE::BlobWithContentType> ResolveTexture(const std::wstring& name)
	{
		auto res = _resourceResovler->ResolveTexture(Platform::StringReference(name.c_str(), name.length()));
		return{ co_await ReadStream(res), std::wstring(res->ContentType->Begin(), res->ContentType->End()) };
	}

	virtual concurrency::task<std::wstring> ResolveMap(const std::wstring& name)
	{
		auto res = _resourceResovler->ResolveMap(Platform::StringReference(name.c_str(), name.length()));
		return ReadString(res);
	}

	virtual concurrency::task<NS_ENGINE::SpritePackageContent> ResolveSpritePackageFile(const std::wstring& name)
	{
		auto res = _resourceResovler->ResolveSpritePackageFile(Platform::StringReference(name.c_str(), name.length()));
		auto image = co_await ReadStream(res->Image);
		auto coordinate = co_await ReadString(res->Coordinate);
		auto sequence = co_await ReadString(res->Sequence);
		return{ image, coordinate, sequence };
	}

	virtual concurrency::task<NS_ENGINE::UnitArt> ResolveUnitArt(const std::wstring& name)
	{
		auto res = _resourceResovler->ResolveUnitArt(Platform::StringReference(name.c_str(), name.length()));
		auto sprite = res->Sprite;
		NS_ENGINE::UnitArt unitArt{ res->Remapable, std::wstring(sprite->Begin(), sprite->End()) };
		for (auto&& offsetSrc : res->WeaponFireOffsets)
			unitArt.WeaponFireOffsets.emplace_back(NS_ENGINE::WeaponFireOffset{ offsetSrc->Forward, offsetSrc->X, offsetSrc->Y });
		return concurrency::task_from_result(unitArt);
	}
private:
	Concurrency::task<std::vector<byte>> ReadStream(Windows::Storage::Streams::IRandomAccessStream^ stream)
	{
		auto dataReader = ref new Windows::Storage::Streams::DataReader(stream->CloneStream());
		auto len = co_await concurrency::create_task(dataReader->LoadAsync(stream->Size));
		std::vector<byte> data(len);
		dataReader->ReadBytes(Platform::ArrayReference<byte>(data.data(), data.size()));
		return std::move(data);
	}

	Concurrency::task<std::vector<byte>> ReadStream(Windows::Storage::Streams::IInputStream^ stream)
	{
		auto dataReader = ref new Windows::Storage::Streams::DataReader(stream);
		std::vector<byte> data;
		while (true)
		{
			auto len = co_await concurrency::create_task(dataReader->LoadAsync(64 * 1024));
			if (len == 0)break;
			auto oldLen = data.size();
			data.resize(oldLen + len);
			dataReader->ReadBytes(Platform::ArrayReference<byte>(data.data() + oldLen, len));
		}
		return std::move(data);
	}

	Concurrency::task<std::wstring> ReadString(Windows::Storage::Streams::IRandomAccessStream^ stream)
	{
		auto dataReader = ref new Windows::Storage::Streams::DataReader(stream->CloneStream());
		auto len = co_await concurrency::create_task(dataReader->LoadAsync(stream->Size));
		auto str = dataReader->ReadString(len);
		if (str->Length() && *str->Begin() == 0x0000FEFFu)
			return std::wstring(str->Begin() + 1, str->End());
		return std::wstring(str->Begin(), str->End());
	}

	Concurrency::task<std::wstring> ReadString(Windows::Storage::Streams::IInputStream^ stream)
	{
		auto dataReader = ref new Windows::Storage::Streams::DataReader(stream);
		std::wstringstream ss;
		while (true)
		{
			auto len = co_await concurrency::create_task(dataReader->LoadAsync(64 * 1024));
			if (len == 0)break;
			auto str = dataReader->ReadString(len);
			if (str->Length() && *str->Begin() == 0x0000FEFFu)
				ss << std::wstring(str->Begin() + 1, str->End());
			else
				ss << std::wstring(str->Begin(), str->End());
		}
		return ss.str();
	}
private:
	IGameEngineResourceResolver^ _resourceResovler;
};

END_NS_RAP