//
// Red Alert Plus Engine
// Terrain Renderer
// 作者：SunnyCase
// 创建时间：2016-05-30
//
#include "pch.h"
#include "TerrainRender.h"
#include <DirectXCollision.h>

using namespace NS_ENGINE;
using namespace WRL;
using namespace DirectX;

namespace
{
	constexpr wchar_t TileLayerVSName[] = L"TileLayerVS";
	constexpr wchar_t TileLayerPSName[] = L"TileLayerPS";

#pragma pack(push, 1)
	struct TileObjectVertex
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT2 texCoordImg;
		DirectX::XMFLOAT2 texCoordExt;
		UINT IsExtra;
	};
#pragma pack(pop)
	static_assert(sizeof(TileObjectVertex) == 32, "Invalid TileObjectVertex Size.");

	static const D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "IsExtra", 0, DXGI_FORMAT_R32_UINT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};
}

TerrainRender::TerrainRender(DeviceContext & deviceContext)
	:_deviceContext(deviceContext)
{
}

static const UINT c_alignedConstantBufferSize = (sizeof(ModelViewProjectionConstantBuffer) + 255) & ~255;

concurrency::task<void> TerrainRender::CreateDeviceDependentResources(IResourceResovler* resourceResolver)
{
	auto d3dDevice = _deviceContext.D3DDevice;

	// 为常量缓冲区创建描述符堆。
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = DeviceContext::FrameCount;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		// 此标志指示此描述符堆可以绑定到管道，并且其中包含的描述符可以由根表引用。
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&_cbvHeap)));
	}

	CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);

	CD3DX12_RESOURCE_DESC constantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(DeviceContext::FrameCount * c_alignedConstantBufferSize);
	ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&constantBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&_constantBuffer)));

	// 创建常量缓冲区视图以访问上载缓冲区。
	D3D12_GPU_VIRTUAL_ADDRESS cbvGpuAddress = _constantBuffer->GetGPUVirtualAddress();
	CD3DX12_CPU_DESCRIPTOR_HANDLE cbvCpuHandle(_cbvHeap->GetCPUDescriptorHandleForHeapStart());
	_cbvDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	for (size_t i = 0; i < DeviceContext::FrameCount; i++)
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
		desc.BufferLocation = cbvGpuAddress;
		desc.SizeInBytes = c_alignedConstantBufferSize;
		d3dDevice->CreateConstantBufferView(&desc, cbvCpuHandle);

		cbvGpuAddress += desc.SizeInBytes;
		cbvCpuHandle.Offset(_cbvDescriptorSize);
	}
	// 映射常量缓冲区。
	CD3DX12_RANGE readRange(0, 0);		// 我们不打算从 CPU 上的此资源中进行读取。
	ThrowIfFailed(_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&_mappedConstantBuffer)));
	ZeroMemory(_mappedConstantBuffer, DeviceContext::FrameCount * c_alignedConstantBufferSize);

	// 创建具有单个常量缓冲区槽的根签名。
	{
		CD3DX12_DESCRIPTOR_RANGE range[2];
		range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		range[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

		CD3DX12_ROOT_PARAMETER rootParameters[2];
		rootParameters[0].InitAsDescriptorTable(1, &range[0], D3D12_SHADER_VISIBILITY_VERTEX);
		rootParameters[1].InitAsDescriptorTable(1, &range[1], D3D12_SHADER_VISIBILITY_ALL);

		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // 只有输入汇编程序阶段才需要访问常量缓冲区。
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

		D3D12_STATIC_SAMPLER_DESC sampler = {};
		sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.MipLODBias = 0;
		sampler.MaxAnisotropy = 0;
		sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		sampler.MinLOD = 0.0f;
		sampler.MaxLOD = D3D12_FLOAT32_MAX;
		sampler.ShaderRegister = 0;
		sampler.RegisterSpace = 0;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
		descRootSignature.Init(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> pSignature;
		ComPtr<ID3DBlob> pError;
		ThrowIfFailed(D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &pSignature, &pError));
		ThrowIfFailed(d3dDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&_rootSignature)));
	}

	ComPtr<IResourceResovler> resourceResovlerHolder(resourceResolver);
	std::array<concurrency::task<std::vector<byte>>, 2> VsPsTasks{
		resourceResovlerHolder->ResovleShader(TileLayerVSName), resourceResovlerHolder->ResovleShader(TileLayerPSName) };
	co_await concurrency::when_all(VsPsTasks.begin(), VsPsTasks.end());
	auto vs = VsPsTasks[0].get();
	auto ps = VsPsTasks[1].get();

	D3D12_GRAPHICS_PIPELINE_STATE_DESC state{};
	state.InputLayout = { inputLayout, _countof(inputLayout) };
	state.pRootSignature = _rootSignature.Get();
	state.VS = CD3DX12_SHADER_BYTECODE(vs.data(), vs.size());
	state.PS = CD3DX12_SHADER_BYTECODE(ps.data(), ps.size());
	state.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

	CD3DX12_BLEND_DESC blendDesc(D3D12_DEFAULT);
	//blendDesc.RenderTarget[0].BlendEnable = TRUE;
	//blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	//blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
	//blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	//blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	//blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	state.BlendState = blendDesc;

	state.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	state.SampleMask = UINT_MAX;
	state.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	state.NumRenderTargets = 1;
	state.RTVFormats[0] = _deviceContext.BackBufferFormat;
	state.DSVFormat = _deviceContext.DepthBufferFormat;
	state.SampleDesc.Count = 1;

	ThrowIfFailed(d3dDevice->CreateGraphicsPipelineState(&state, IID_PPV_ARGS(&_pipelineState)));
	_deviceContext.CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _pipelineState.Get(), IID_PPV_ARGS(&_commandList));
	ThrowIfFailed(_commandList->Close());

	auto tileSetRes = co_await resourceResolver->ResolveTileSetPackageFile(_mapInfo->tileSet);
	_tileSetImageTex = _deviceContext.TextureManager.CreateTexture2D(tileSetRes.Image.data(), tileSetRes.Image.size(), L"image/png");
	_tileSetExtraImageTex = _deviceContext.TextureManager.CreateTexture2D(tileSetRes.ExtraImage.data(), tileSetRes.ExtraImage.size(), L"image/png");

	_tileSetInfo = std::make_unique<TileSetInfo>(tileSetRes.TileSet, _tileSetImageTex.Width, _tileSetImageTex.Height,
		_tileSetExtraImageTex.Width, _tileSetExtraImageTex.Height);
	CreateTerrainTree();
}

concurrency::task<void> TerrainRender::CreateWindowSizeDependentResources(IResourceResovler * resourceResolver)
{
	D3D12_VIEWPORT viewport = _deviceContext.ScreenViewport;
	_scissorRect = { 0, 0, static_cast<LONG>(viewport.Width), static_cast<LONG>(viewport.Height) };

	XMMATRIX orthoMat = DirectX::XMMatrixOrthographicRH(viewport.Width, viewport.Height, viewport.MinDepth, viewport.MaxDepth);
	XMStoreFloat4x4(
		&_constantData.projection,
		XMMatrixTranspose(orthoMat)
	);

	return concurrency::task_from_result();
}

void TerrainRender::UploadGpuResource(std::vector<WRL::ComPtr<IUnknown>>& resourcesWaitForUpload)
{
	ComPtr<ID3D12GraphicsCommandList> commandList;
	_deviceContext.CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _pipelineState.Get(), IID_PPV_ARGS(&commandList));

	_deviceContext.TextureManager.Upload(_tileSetImageTex, commandList.Get(), resourcesWaitForUpload);
	_deviceContext.TextureManager.Upload(_tileSetExtraImageTex, commandList.Get(), resourcesWaitForUpload);

	_terrainTree.ForEach([&](auto& node) { return node.NodeManager.UploadGpuResource(commandList.Get(), resourcesWaitForUpload); });
	ThrowIfFailed(commandList->Close());
	_deviceContext.ExecuteCommandList(commandList.Get());
}

void TerrainRender::Update()
{
	const float left = _currentMargin.x, bottom = _currentMargin.y;

	D3D12_VIEWPORT viewport = _deviceContext.ScreenViewport;

	const XMVECTORF32 eye = { left + viewport.Width / 2.f, bottom + viewport.Height / 2.f, 1.0f, 0.0f };
	const XMVECTORF32 at = { eye.f[0], eye.f[1], 0.0f, 0.0f };
	const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4(&_constantData.view, XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));

	_currentViewRect = { left, bottom + viewport.Height, left + viewport.Width, bottom };

	// 更新常量缓冲区资源。
	UINT8* destination = _mappedConstantBuffer + (_deviceContext.CurrentFrameIndex * c_alignedConstantBufferSize);
	memcpy_s(destination, sizeof(_constantData), &_constantData, sizeof(_constantData));
}

void TerrainRender::Render()
{
	ThrowIfFailed(_commandList->Reset(_deviceContext.CurrentCommandAllocator, _pipelineState.Get()));
	PIXBeginEvent(_commandList.Get(), 0, L"Draw the cube");
	{
		// 设置要由此帧使用的图形根签名和描述符堆。
		_commandList->SetPipelineState(_pipelineState.Get());
		_commandList->SetGraphicsRootSignature(_rootSignature.Get());
		ID3D12DescriptorHeap* descHeaps[] = { _deviceContext.TextureManager.GetHeap() };

		// 设置视区和剪刀矩形。
		auto& viewport = _deviceContext.ScreenViewport;
		_commandList->RSSetViewports(1, &viewport);
		_commandList->RSSetScissorRects(1, &_scissorRect);

		_commandList->SetDescriptorHeaps(1, _cbvHeap.GetAddressOf());
		// 将当前帧的常量缓冲区绑定到管道。
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(_cbvHeap->GetGPUDescriptorHandleForHeapStart(), _deviceContext.CurrentFrameIndex, _cbvDescriptorSize);
		_commandList->SetGraphicsRootDescriptorTable(0, gpuHandle);
		//_commandList->SetGraphicsRootConstantBufferView(0, gpuHandle);

		// 指示此资源会用作呈现目标。
		CD3DX12_RESOURCE_BARRIER renderTargetResourceBarrier =
			CD3DX12_RESOURCE_BARRIER::Transition(_deviceContext.RenderTarget, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		_commandList->ResourceBarrier(1, &renderTargetResourceBarrier);

		// 记录绘制命令。
		D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView = _deviceContext.RenderTargetView;
		D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView = _deviceContext.DepthStencilView;

		_commandList->OMSetRenderTargets(1, &renderTargetView, false, &depthStencilView);

		_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		_commandList->SetDescriptorHeaps(1, descHeaps);
		_commandList->SetGraphicsRootDescriptorTable(1, _tileSetImageTex.GPUHandle);

		std::vector<TerrianNode_t*> nodes;
		_terrainTree.HitTest(nodes, _currentViewRect);
		for (auto&& node : nodes)
			node->NodeManager.Render(_commandList.Get());

		// 指示呈现目标现在会用于展示命令列表完成执行的时间。
		CD3DX12_RESOURCE_BARRIER presentResourceBarrier =
			CD3DX12_RESOURCE_BARRIER::Transition(_deviceContext.RenderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		_commandList->ResourceBarrier(1, &presentResourceBarrier);
	}
	PIXEndEvent(_commandList.Get());
	ThrowIfFailed(_commandList->Close());
	_deviceContext.ExecuteCommandList(_commandList.Get());
}

void TerrainRender::SetMap(std::shared_ptr<MapInfo> mapInfo)
{
	_mapInfo = std::move(mapInfo);
}

namespace
{
	uint32_t CalcMapHeight(uint32_t rows, uint32_t tileHeight)
	{
		if (!rows) return 0;
		return rows % 2 ? (rows / 2 + 1) * tileHeight : (rows / 2) * tileHeight + tileHeight / 2;
	}
}

void TerrainRender::CreateTerrainTree()
{
	const auto tileWidth = _tileSetInfo->TileWidth;
	const auto tileHeight = _tileSetInfo->TileHeight;
	const auto heightShift = tileHeight / 2;
	const auto widthShift = tileWidth / 2;

	_terrainTree.BoundingRect = { 0, float(CalcMapHeight(_mapInfo->height, tileHeight)), float(_mapInfo->width * tileWidth + widthShift), 0 };

	bool evenRow = true;
	uint32_t cntY = 0;
	for (auto&& row : _mapInfo->tiles)
	{
		uint32_t cntX = evenRow ? widthShift : 0;
		for (auto&& cell : row)
		{
			const auto heightedY = cntY + heightShift * cell.second;
			auto& tile = _tileSetInfo->FindTile(cell.first);
			TileObject obj;
			obj.ImageSource = ImageSourceKind::Image;
			obj.ProjectedRect = tile.Rect.MakeOffset(cntX, heightedY);
			obj.LeftTopUV = tile.LeftTopUV;
			obj.RightBottomUV = tile.RightBottomUV;
			_terrainTree.Add(obj, obj.ProjectedRect);

			const auto extra = tile.ExtraImg;
			if (extra)
			{
				TileObject extObj;
				extObj.ImageSource = ImageSourceKind::ExtraImage;
				extObj.ProjectedRect = extra->Rect.MakeOffset(tile.ExtraImgOffset).MakeOffset(cntX, heightedY);
				extObj.LeftTopUV = extra->LeftTopUV;
				extObj.RightBottomUV = extra->RightBottomUV;
				_terrainTree.Add(extObj, extObj.ProjectedRect);
			}
			cntX += tileWidth;
		}
		cntY += heightShift;
		evenRow = !evenRow;
	}
	_terrainTree.ForEach([=](auto& node)
	{
		node.NodeManager.SetDevice(_deviceContext);
		node.NodeManager.SetTexture(_tileSetImageTex, _tileSetExtraImageTex);
		node.Sealed = true;
	});
}

DirectX::XMFLOAT2 TerrainRender::SetMapMargin(const DirectX::XMFLOAT2 & margin)
{
	auto viewport = _deviceContext.ScreenViewport;
	XMFLOAT2 maxMargin = { _terrainTree.BoundingRect.RightBottom.x - viewport.Width, _terrainTree.BoundingRect.LeftTop.y - viewport.Height };
	XMFLOAT2 newMargin = margin;
	newMargin.x = std::max(0.f, std::min(newMargin.x, maxMargin.x));
	newMargin.y = std::max(0.f, std::min(newMargin.y, maxMargin.y));

	_currentMargin = newMargin;
	return newMargin;
}

void TerrainRender::TerrianNodeRender::UploadGpuResource(ID3D12GraphicsCommandList* commandList, std::vector<WRL::ComPtr<IUnknown>>& resourcesWaitForUpload)
{
	const auto& objects = _node->Objects;
	if (objects.empty()) return;

	std::vector<TileObjectVertex> vertices;
	vertices.reserve(objects.size() * 4);
	std::vector<uint16_t> indices;
	indices.reserve(objects.size() * 6);

	uint16_t idxBase = 0;
	for (auto&& obj : objects)
	{
		UINT isExtra = obj.ImageSource == ImageSourceKind::ExtraImage ? 1 : 0;
		const auto& rect = obj.ProjectedRect;
		vertices.emplace_back<TileObjectVertex>(
		{
			{ rect.LeftTop.x, rect.RightBottom.y, 0.5f },
			{ obj.LeftTopUV.x, obj.RightBottomUV.y },
			{ obj.LeftTopUV.x, obj.RightBottomUV.y },
			isExtra
		});
		vertices.emplace_back<TileObjectVertex>(
		{
			{ rect.LeftTop.x, rect.LeftTop.y, 0.5f },
			{ obj.LeftTopUV.x, obj.LeftTopUV.y },
			{ obj.LeftTopUV.x, obj.LeftTopUV.y },
			isExtra
		});
		vertices.emplace_back<TileObjectVertex>(
		{
			{ rect.RightBottom.x, rect.LeftTop.y, 0.5f },
			{ obj.RightBottomUV.x, obj.LeftTopUV.y },
			{ obj.RightBottomUV.x, obj.LeftTopUV.y },
			isExtra
		});
		vertices.emplace_back<TileObjectVertex>(
		{
			{ rect.RightBottom.x, rect.RightBottom.y, 0.5f },
			{ obj.RightBottomUV.x, obj.RightBottomUV.y },
			{ obj.RightBottomUV.x, obj.RightBottomUV.y },
			isExtra
		});
		indices.emplace_back(idxBase + 0);
		indices.emplace_back(idxBase + 1);
		indices.emplace_back(idxBase + 2);

		indices.emplace_back(idxBase + 0);
		indices.emplace_back(idxBase + 2);
		indices.emplace_back(idxBase + 3);
		idxBase += 4;
	}
	auto d3dDevice = _deviceContext->D3DDevice;
	const UINT vertexBufferSize = vertices.size() * sizeof(TileObjectVertex);
	// 在 GPU 的默认堆中创建顶点缓冲区资源并使用上载堆将顶点数据复制到其中。
	// 在 GPU 使用完之前，不得释放上载资源。
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferUpload;

	CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC vertexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
	ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&defaultHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&vertexBufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&_vertexBuffer)));

	CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
	ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&vertexBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertexBufferUpload)));
	// 将顶点缓冲区上载到 GPU。
	{
		D3D12_SUBRESOURCE_DATA vertexData = {};
		vertexData.pData = reinterpret_cast<BYTE*>(vertices.data());
		vertexData.RowPitch = vertexBufferSize;
		vertexData.SlicePitch = vertexData.RowPitch;

		UpdateSubresources(commandList, _vertexBuffer.Get(), vertexBufferUpload.Get(), 0, 0, 1, &vertexData);

		CD3DX12_RESOURCE_BARRIER vertexBufferResourceBarrier =
			CD3DX12_RESOURCE_BARRIER::Transition(_vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		commandList->ResourceBarrier(1, &vertexBufferResourceBarrier);
	}
	resourcesWaitForUpload.emplace_back(vertexBufferUpload);

	const UINT indexBufferSize = indices.size() * sizeof(uint16_t);

	// 在 GPU 的默认堆中创建索引缓冲区资源并使用上载堆将索引数据复制到其中。
	// 在 GPU 使用完之前，不得释放上载资源。
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBufferUpload;

	CD3DX12_RESOURCE_DESC indexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);
	ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&defaultHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&indexBufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&_indexBuffer)));

	ThrowIfFailed(d3dDevice->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&indexBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBufferUpload)));

	// 将索引缓冲区上载到 GPU。
	{
		D3D12_SUBRESOURCE_DATA indexData = {};
		indexData.pData = reinterpret_cast<BYTE*>(indices.data());
		indexData.RowPitch = indexBufferSize;
		indexData.SlicePitch = indexData.RowPitch;

		UpdateSubresources(commandList, _indexBuffer.Get(), indexBufferUpload.Get(), 0, 0, 1, &indexData);

		CD3DX12_RESOURCE_BARRIER indexBufferResourceBarrier =
			CD3DX12_RESOURCE_BARRIER::Transition(_indexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
		commandList->ResourceBarrier(1, &indexBufferResourceBarrier);
	}
	resourcesWaitForUpload.emplace_back(indexBufferUpload);
	// 创建顶点/索引缓冲区视图。
	_vertexBufferView.BufferLocation = _vertexBuffer->GetGPUVirtualAddress();
	_vertexBufferView.StrideInBytes = sizeof(TileObjectVertex);
	_vertexBufferView.SizeInBytes = vertexBufferSize;

	_indexBufferView.BufferLocation = _indexBuffer->GetGPUVirtualAddress();
	_indexBufferView.SizeInBytes = indexBufferSize;
	_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
}

void TerrainRender::TerrianNodeRender::Render(ID3D12GraphicsCommandList * commandList)
{
	if (_node->Objects.empty()) return;

	commandList->IASetVertexBuffers(0, 1, &_vertexBufferView);
	commandList->IASetIndexBuffer(&_indexBufferView);
	commandList->DrawIndexedInstanced(_node->Objects.size() * 6, 1, 0, 0, 0);
}
