//
// Red Alert Plus Engine
// Terrain Renderer
// ���ߣ�SunnyCase
// ����ʱ�䣺2016-05-30
//
#include "pch.h"
#include "TerrainRender.h"

using namespace NS_ENGINE;
using namespace WRL;

namespace
{
	constexpr wchar_t TileLayerVSName[] = L"TileLayerVS";
	constexpr wchar_t TileLayerPSName[] = L"TileLayerPS";
}

TerrainRender::TerrainRender(DeviceContext & deviceContext)
	:_deviceContext(deviceContext)
{
}

concurrency::task<void> TerrainRender::CreateDeviceDependentResources(IResourceResovler* resourceResolver)
{
	auto d3dDevice = _deviceContext.D3DDevice;
	
	// �������е��������������۵ĸ�ǩ����
	{
		CD3DX12_DESCRIPTOR_RANGE range;
		CD3DX12_ROOT_PARAMETER parameter;

		range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		parameter.InitAsDescriptorTable(1, &range, D3D12_SHADER_VISIBILITY_VERTEX);

		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // ֻ�����������׶β���Ҫ���ʳ�����������
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

		CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
		descRootSignature.Init(1, &parameter, 0, nullptr, rootSignatureFlags);

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

	static const D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC state{};
	state.InputLayout = { inputLayout, _countof(inputLayout) };
	state.pRootSignature = _rootSignature.Get();
	state.VS = CD3DX12_SHADER_BYTECODE(vs.data(), vs.size());
	state.PS = CD3DX12_SHADER_BYTECODE(ps.data(), ps.size());
	state.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	state.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
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
}

concurrency::task<void> TerrainRender::CreateWindowSizeDependentResources(IResourceResovler * resourceResolver)
{
	D3D12_VIEWPORT viewport = _deviceContext.ScreenViewport;
	_scissorRect = { 0, 0, static_cast<LONG>(viewport.Width), static_cast<LONG>(viewport.Height) };

	return concurrency::task_from_result();
}


// �����򶥵���ɫ������ MVP ����ĳ�����������
struct ModelViewProjectionConstantBuffer
{
	DirectX::XMFLOAT4X4 model;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
};

// �����򶥵���ɫ������ÿ����������ݡ�
struct VertexPositionColor
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 color;
};

using namespace DirectX;

void TerrainRender::UploadGpuResource(std::vector<WRL::ComPtr<IUnknown>>& resourcesWaitForUpload)
{
	ComPtr<ID3D12GraphicsCommandList> commandList;
	_deviceContext.CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _pipelineState.Get(), IID_PPV_ARGS(&commandList));

	// �����嶥�㡣ÿ�����㶼��һ��λ�ú�һ����ɫ��
	VertexPositionColor cubeVertices[] =
	{
		{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(0.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(0.5f, -0.5f,  0.5f), XMFLOAT3(1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(0.5f,  0.5f, -0.5f), XMFLOAT3(1.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(0.5f,  0.5f,  0.5f), XMFLOAT3(1.0f, 1.0f, 1.0f) },
	};

	auto d3dDevice = _deviceContext.D3DDevice;
	const UINT vertexBufferSize = sizeof(cubeVertices);
	// �� GPU ��Ĭ�϶��д������㻺������Դ��ʹ�����ضѽ��������ݸ��Ƶ����С�
	// �� GPU ʹ����֮ǰ�������ͷ�������Դ��
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
	// �����㻺�������ص� GPU��
	{
		D3D12_SUBRESOURCE_DATA vertexData = {};
		vertexData.pData = reinterpret_cast<BYTE*>(cubeVertices);
		vertexData.RowPitch = vertexBufferSize;
		vertexData.SlicePitch = vertexData.RowPitch;

		UpdateSubresources(commandList.Get(), _vertexBuffer.Get(), vertexBufferUpload.Get(), 0, 0, 1, &vertexData);

		CD3DX12_RESOURCE_BARRIER vertexBufferResourceBarrier =
			CD3DX12_RESOURCE_BARRIER::Transition(_vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		commandList->ResourceBarrier(1, &vertexBufferResourceBarrier);
	}
	resourcesWaitForUpload.emplace_back(vertexBufferUpload);
	// ��������������ÿ����������ʾҪ����Ļ�ϳ��ֵ������Ρ�
	// ����: 0,2,1 ��ʾ���㻺�����е�����Ϊ 0��2 �� 1 �Ķ��㹹��
	// ������ĵ�һ�������Ρ�
	unsigned short cubeIndices[] =
	{
		0, 2, 1, // -x
		1, 2, 3,

		4, 5, 6, // +x
		5, 7, 6,

		0, 1, 5, // -y
		0, 5, 4,

		2, 6, 7, // +y
		2, 7, 3,

		0, 4, 6, // -z
		0, 6, 2,

		1, 3, 7, // +z
		1, 7, 5,
	};

	const UINT indexBufferSize = sizeof(cubeIndices);

	// �� GPU ��Ĭ�϶��д���������������Դ��ʹ�����ضѽ��������ݸ��Ƶ����С�
	// �� GPU ʹ����֮ǰ�������ͷ�������Դ��
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

	// ���������������ص� GPU��
	{
		D3D12_SUBRESOURCE_DATA indexData = {};
		indexData.pData = reinterpret_cast<BYTE*>(cubeIndices);
		indexData.RowPitch = indexBufferSize;
		indexData.SlicePitch = indexData.RowPitch;

		UpdateSubresources(commandList.Get(), _indexBuffer.Get(), indexBufferUpload.Get(), 0, 0, 1, &indexData);

		CD3DX12_RESOURCE_BARRIER indexBufferResourceBarrier =
			CD3DX12_RESOURCE_BARRIER::Transition(_indexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
		commandList->ResourceBarrier(1, &indexBufferResourceBarrier);
	}
	resourcesWaitForUpload.emplace_back(indexBufferUpload);

	_deviceContext.TextureManager.Upload(_tileSetImageTex, commandList.Get(), resourcesWaitForUpload);
	_deviceContext.TextureManager.Upload(_tileSetExtraImageTex, commandList.Get(), resourcesWaitForUpload);

	ThrowIfFailed(commandList->Close());
	_deviceContext.ExecuteCommandList(commandList.Get());
	// ��������/������������ͼ��
	_vertexBufferView.BufferLocation = _vertexBuffer->GetGPUVirtualAddress();
	_vertexBufferView.StrideInBytes = sizeof(VertexPositionColor);
	_vertexBufferView.SizeInBytes = sizeof(cubeVertices);

	_indexBufferView.BufferLocation = _indexBuffer->GetGPUVirtualAddress();
	_indexBufferView.SizeInBytes = sizeof(cubeIndices);
	_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
}

void TerrainRender::Update()
{
}

void TerrainRender::Render()
{
	ThrowIfFailed(_commandList->Reset(_deviceContext.CurrentCommandAllocator, _pipelineState.Get()));
	PIXBeginEvent(_commandList.Get(), 0, L"Draw the cube");
	{
		// ����Ҫ�ɴ�֡ʹ�õ�ͼ�θ�ǩ�����������ѡ�
		_commandList->SetGraphicsRootSignature(_rootSignature.Get());

		// ���������ͼ������Ρ�
		auto& viewport = _deviceContext.ScreenViewport;
		_commandList->RSSetViewports(1, &viewport);
		_commandList->RSSetScissorRects(1, &_scissorRect);

		// ָʾ����Դ����������Ŀ�ꡣ
		CD3DX12_RESOURCE_BARRIER renderTargetResourceBarrier =
			CD3DX12_RESOURCE_BARRIER::Transition(_deviceContext.RenderTarget, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		_commandList->ResourceBarrier(1, &renderTargetResourceBarrier);

		// ��¼�������
		D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView = _deviceContext.RenderTargetView;
		D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView = _deviceContext.DepthStencilView;
		_commandList->ClearRenderTargetView(renderTargetView, DirectX::Colors::CornflowerBlue, 0, nullptr);
		_commandList->ClearDepthStencilView(depthStencilView, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		_commandList->OMSetRenderTargets(1, &renderTargetView, false, &depthStencilView);

		_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		_commandList->IASetVertexBuffers(0, 1, &_vertexBufferView);
		_commandList->IASetIndexBuffer(&_indexBufferView);
		_commandList->DrawIndexedInstanced(36, 1, 0, 0, 0);

		// ָʾ����Ŀ�����ڻ�����չʾ�����б����ִ�е�ʱ�䡣
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
