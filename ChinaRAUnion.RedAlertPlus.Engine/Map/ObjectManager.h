//
// Red Alert Plus Engine
// ObjectManager
// 作者：SunnyCase
// 创建时间：2016-06-15
//
#pragma once
#include "../../include/engine/engine.h"
#include "../IDeviceDependentResourcesContainer.h"
#include "../IRenderable.h"
#include "../DeviceContext.h"
#include <ppltasks.h>
#include "ModelViewProjectionConstantBuffer.h"
#include "../Math/Collision.h"
#include "../Config/SpriteReader.h"

DEFINE_NS_ENGINE

class AsyncArtContainer
{
public:
	AsyncArtContainer(IResourceResovler* resourceResolver);


private:
	WRL::ComPtr<IResourceResovler> _resourceResolver;
};

class ObjectManager : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::WinRtClassicComMix>, IInspectable, NS_RAP::Api::IObjectManager>, 
	public IDeviceDependentResourcesContainer, public IRenderable
{
#pragma pack(push, 1)
	struct SpriteVertex
	{
		DirectX::XMFLOAT3 pos;
		UINT Remapable;
	};

	static_assert(sizeof(SpriteVertex) == 16, "Invalid SpriteVertex Size.");
#pragma pack(pop)

	class SpriteObject
	{
	public:
		SpriteObject(DeviceContext& deviceContext, Texture& texture, std::shared_ptr<SpriteCoordinateReader>& coordinate, std::shared_ptr<SpriteSequenceReader>& sequence);

		const std::vector<SpriteVertex>& GetVertices() const noexcept { return _vertices; }
	private:
		DeviceContext& _deviceContext;
		Texture& _texture;
		std::vector<SpriteVertex> _vertices;
		std::shared_ptr<SpriteCoordinateReader> _coordinate;
		std::shared_ptr<SpriteSequenceReader> _sequence;
	};

	class SpriteBatch
	{
	public:
		SpriteBatch(DeviceContext& deviceContext, Texture& texture, std::shared_ptr<SpriteCoordinateReader>& coordinate, std::shared_ptr<SpriteSequenceReader>& sequence);

		void Update();
		void Render(ID3D12GraphicsCommandList* commandList);
		size_t Attach(SpriteObject&& object);
		void Detach(size_t id);
	private:
		DeviceContext& _deviceContext;
		Texture& _texture;
		std::shared_ptr<SpriteCoordinateReader> _coordinate;
		std::shared_ptr<SpriteSequenceReader> _sequence;
		std::unordered_map<size_t, SpriteObject> _sprites;
		WRL::ComPtr<ID3D12Resource> _vertexBuffer;
	};
public:
	ObjectManager(DeviceContext& deviceContext, IResourceResovler* resourceResolver, IRulesResolver* rulesResolver);

	virtual concurrency::task<void> CreateDeviceDependentResources(IResourceResovler* resourceResolver);
	virtual concurrency::task<void> CreateWindowSizeDependentResources(IResourceResovler* resourceResolver);
	virtual void UploadGpuResource(std::vector<WRL::ComPtr<IUnknown>>& resourcesWaitForUpload);

	virtual void Update();
	virtual void Render();

	STDMETHODIMP raw_PlaceInfantry(BSTR name, ULONG x, ULONG y, VARIANT_BOOL * pRetVal) override;
private:
private:
	DeviceContext& _deviceContext;
	WRL::ComPtr<IResourceResovler> _resourceResolver;
	WRL::ComPtr<IRulesResolver> _rulesResolver;

	WRL::ComPtr<ID3D12RootSignature> _rootSignature;
	WRL::ComPtr<ID3D12PipelineState> _pipelineState;
	WRL::ComPtr<ID3D12GraphicsCommandList> _commandList;
	WRL::ComPtr<ID3D12DescriptorHeap> _cbvHeap;
	WRL::ComPtr<ID3D12Resource> _constantBuffer;
	std::unordered_map<std::wstring, SpriteBatch> _spriteBatches;
	UINT32 _cbvDescriptorSize;
	byte* _mappedConstantBuffer;

	ModelViewProjectionConstantBuffer _constantData;

	D3D12_RECT _scissorRect;
	Tomato::Rect _currentViewRect;
	DirectX::XMFLOAT2 _currentMargin = {};
	Texture _giTexture;
};

END_NS_ENGINE