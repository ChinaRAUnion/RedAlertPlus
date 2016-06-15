//
// Red Alert Plus Engine
// ObjectManager
// 作者：SunnyCase
// 创建时间：2016-06-15
//
#include "pch.h"
#include "ObjectManager.h"
#include "../Config/SpriteReader.h"

using namespace NS_ENGINE;
using namespace WRL;
using namespace DirectX;

namespace
{
	constexpr wchar_t SpriteVSName[] = L"SpriteVS";
	constexpr wchar_t SpritePSName[] = L"SpritePS";

#pragma pack(push, 1)
	struct SpriteVertex
	{
		DirectX::XMFLOAT3 pos;
		UINT Id;
		UINT Remapable;
	};
#pragma pack(pop)
	static_assert(sizeof(SpriteVertex) == 20, "Invalid SpriteVertex Size.");

	static const D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "Id", 0, DXGI_FORMAT_R32_UINT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "Remapable", 0, DXGI_FORMAT_R32_UINT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	static const UINT c_alignedConstantBufferSize = (sizeof(ModelViewProjectionConstantBuffer) + 255) & ~255;
}

ObjectManager::ObjectManager(DeviceContext & deviceContext, IResourceResovler* resourceResolver, IRulesResolver* rulesResolver)
	:_deviceContext(deviceContext), _resourceResolver(resourceResolver), _rulesResolver(rulesResolver)
{
}

concurrency::task<void> ObjectManager::CreateDeviceDependentResources(IResourceResovler * resourceResolver)
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
		resourceResovlerHolder->ResovleShader(SpriteVSName), resourceResovlerHolder->ResovleShader(SpritePSName) };
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

	auto art = co_await resourceResolver->ResolveUnitArt(L"GI");
	auto sprite = co_await resourceResolver->ResolveSpritePackageFile(art.Sprite);
	_giTexture = _deviceContext.TextureManager.CreateTexture2D(sprite.Image.data(), sprite.Image.size(), L"image/dds");
	
	SpriteCoordinateReader coordinateReader(_giTexture.Width, _giTexture.Height, sprite.Coordinate);
}

concurrency::task<void> ObjectManager::CreateWindowSizeDependentResources(IResourceResovler * resourceResolver)
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

void ObjectManager::UploadGpuResource(std::vector<WRL::ComPtr<IUnknown>>& resourcesWaitForUpload)
{
	ComPtr<ID3D12GraphicsCommandList> commandList;
	_deviceContext.CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, nullptr, IID_PPV_ARGS(&commandList));

	_deviceContext.TextureManager.Upload(_giTexture, commandList.Get(), resourcesWaitForUpload);

	ThrowIfFailed(commandList->Close());
	_deviceContext.ExecuteCommandList(commandList.Get());
}

void ObjectManager::Update()
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

void ObjectManager::Render()
{
	ThrowIfFailed(_commandList->Reset(_deviceContext.CurrentCommandAllocator, _pipelineState.Get()));
	PIXBeginEvent(_commandList.Get(), 0, L"Draw the cube");
	{
		// 设置要由此帧使用的图形根签名和描述符堆。
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
		_commandList->ClearRenderTargetView(renderTargetView, DirectX::Colors::CornflowerBlue, 0, nullptr);
		_commandList->ClearDepthStencilView(depthStencilView, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		_commandList->OMSetRenderTargets(1, &renderTargetView, false, &depthStencilView);

		_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		_commandList->SetDescriptorHeaps(1, descHeaps);
		_commandList->SetGraphicsRootDescriptorTable(1, _giTexture.GPUHandle);

		//std::vector<TerrianNode_t*> nodes;
		//_terrainTree.HitTest(nodes, _currentViewRect);
		//for (auto&& node : nodes)
		//	node->NodeManager.Render(_commandList.Get());

		// 指示呈现目标现在会用于展示命令列表完成执行的时间。
		CD3DX12_RESOURCE_BARRIER presentResourceBarrier =
			CD3DX12_RESOURCE_BARRIER::Transition(_deviceContext.RenderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		_commandList->ResourceBarrier(1, &presentResourceBarrier);
	}
	PIXEndEvent(_commandList.Get());
	ThrowIfFailed(_commandList->Close());
	_deviceContext.ExecuteCommandList(_commandList.Get());
}

STDMETHODIMP ObjectManager::raw_PlaceInfantry(BSTR name, ULONG x, ULONG y, VARIANT_BOOL * pRetVal)
{
	try
	{
		auto rule = _rulesResolver->FindInfantry(name);
		ThrowIfNot(rule, _bstr_t(L"Cannot find infantry rule: ") + name + L".");
		return S_OK;
	}
	CATCH_ALL();
}
