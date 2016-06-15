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
public:
	ObjectManager(DeviceContext& deviceContext, IResourceResovler* resourceResolver, IRulesResolver* rulesResolver);

	virtual concurrency::task<void> CreateDeviceDependentResources(IResourceResovler* resourceResolver);
	virtual concurrency::task<void> CreateWindowSizeDependentResources(IResourceResovler* resourceResolver);
	virtual void UploadGpuResource(std::vector<WRL::ComPtr<IUnknown>>& resourcesWaitForUpload);

	virtual void Update();
	virtual void Render();

	STDMETHODIMP raw_PlaceInfantry(BSTR name, ULONG x, ULONG y, VARIANT_BOOL * pRetVal) override;
private:
	DeviceContext& _deviceContext;
	WRL::ComPtr<IResourceResovler> _resourceResolver;
	WRL::ComPtr<IRulesResolver> _rulesResolver;

	WRL::ComPtr<ID3D12RootSignature> _rootSignature;
	WRL::ComPtr<ID3D12PipelineState> _pipelineState;
	WRL::ComPtr<ID3D12GraphicsCommandList> _commandList;
	WRL::ComPtr<ID3D12DescriptorHeap> _cbvHeap;
	WRL::ComPtr<ID3D12Resource> _constantBuffer;
	UINT32 _cbvDescriptorSize;
	byte* _mappedConstantBuffer;

	ModelViewProjectionConstantBuffer _constantData;

	D3D12_RECT _scissorRect;
	Tomato::Rect _currentViewRect;
	DirectX::XMFLOAT2 _currentMargin = {};
	Texture _giTexture;
};

END_NS_ENGINE