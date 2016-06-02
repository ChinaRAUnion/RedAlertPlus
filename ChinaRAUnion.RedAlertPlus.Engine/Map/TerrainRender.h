//
// Red Alert Plus Engine
// Terrain Renderer
// 作者：SunnyCase
// 创建时间：2016-05-30
//
#pragma once
#include "DeviceContext.h"
#include "IDeviceDependentResourcesContainer.h"
#include "IRenderable.h"
#include "MapInfo.h"
#include "../Math/Collision.h"
#include "TileSetInfo.h"

DEFINE_NS_ENGINE

// 用于向顶点着色器发送 MVP 矩阵的常量缓冲区。
struct ModelViewProjectionConstantBuffer
{
	DirectX::XMFLOAT4X4 model;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
};

class TerrainRender : public IDeviceDependentResourcesContainer, public IRenderable
{
	enum class ImageSourceKind
	{
		Image, ExtraImage
	};

	struct TileObject
	{
		Tomato::Rect ProjectedRect;
		ImageSourceKind ImageSource;
		DirectX::XMFLOAT2 LeftTopUV, RightBottomUV;
	};

	struct TerrianContinueDivideFn
	{
		bool operator()(const Tomato::Rect& rect, size_t level) const noexcept
		{
			return rect.Width > 5 * 60 && rect.Height > 5 * 30;
		}
	};

	class TerrianNodeRender;
	typedef Tomato::QuadTreeNode<TileObject, TerrianNodeRender, TerrianContinueDivideFn> TerrianNode_t;

	class TerrianNodeRender
	{
	public:

		void SetNode(TerrianNode_t& node)
		{
			_node = &node;
		}

		void SetDevice(DeviceContext& deviceContext)
		{
			_deviceContext = &deviceContext;
		}

		void SetTexture(const Texture& tileSetImage, const Texture& tileSetExtraImage)
		{
			_tileSetImage = &tileSetImage;
			_tileSetExtraImage = &tileSetExtraImage;
		}

		void UploadGpuResource(ID3D12GraphicsCommandList* commandList, std::vector<WRL::ComPtr<IUnknown>>& resourcesWaitForUpload);
		void Render(ID3D12GraphicsCommandList* commandList);
	private:
		TerrianNode_t* _node = nullptr;
		DeviceContext* _deviceContext = nullptr;
		const Texture* _tileSetImage = nullptr;
		const Texture* _tileSetExtraImage = nullptr;
		WRL::ComPtr<ID3D12Resource> _vertexBuffer;
		WRL::ComPtr<ID3D12Resource> _indexBuffer;
		D3D12_VERTEX_BUFFER_VIEW _vertexBufferView;
		D3D12_INDEX_BUFFER_VIEW _indexBufferView;
	};
public:
	TerrainRender(DeviceContext& deviceContext);

	virtual concurrency::task<void> CreateDeviceDependentResources(IResourceResovler* resourceResolver);
	virtual concurrency::task<void> CreateWindowSizeDependentResources(IResourceResovler* resourceResolver);
	virtual void UploadGpuResource(std::vector<WRL::ComPtr<IUnknown>>& resourcesWaitForUpload);
	virtual void Update();
	virtual void Render();

	void SetMap(std::shared_ptr<MapInfo> mapInfo);
	void CreateTerrainTree();

	DirectX::XMFLOAT2 SetMapMargin(const DirectX::XMFLOAT2& margin);
private:
	DeviceContext& _deviceContext;
	std::shared_ptr<MapInfo> _mapInfo;
	std::unique_ptr<TileSetInfo> _tileSetInfo;
	WRL::ComPtr<ID3D12RootSignature> _rootSignature;
	WRL::ComPtr<ID3D12PipelineState> _pipelineState;
	WRL::ComPtr<ID3D12GraphicsCommandList> _commandList;
	WRL::ComPtr<ID3D12DescriptorHeap> _cbvHeap;
	WRL::ComPtr<ID3D12Resource> _constantBuffer;
	UINT32 _cbvDescriptorSize;
	byte* _mappedConstantBuffer;

	Texture _tileSetImageTex, _tileSetExtraImageTex;
	TerrianNode_t _terrainTree;
	ModelViewProjectionConstantBuffer _constantData;

	D3D12_RECT _scissorRect;
	Tomato::Rect _currentViewRect;
	DirectX::XMFLOAT2 _currentMargin = {};
};

END_NS_ENGINE