//
// Red Alert Plus Engine
// Map Renderer
// 作者：SunnyCase
// 创建时间：2016-05-25
//
#include "pch.h"
#include "MapRenderer.h"

using namespace NS_ENGINE;
using namespace WRL;

namespace
{
	constexpr wchar_t TileLayerVSName[] = L"TileLayerVS.cso";
	constexpr wchar_t TileLayerPSName[] = L"TileLayerPS.cso";
}

MapRenderer::MapRenderer(DeviceContext & deviceContext)
	:_deviceContext(deviceContext)
{
}

concurrency::task<void> MapRenderer::CreateDeviceDependentResources(IResourceResovler* resourceResolver)
{
	auto d3dDevice = _deviceContext.D3DDevice;
	// 创建具有单个常量缓冲区槽的根签名。
	{
		CD3DX12_DESCRIPTOR_RANGE range;
		CD3DX12_ROOT_PARAMETER parameter;

		range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		parameter.InitAsDescriptorTable(1, &range, D3D12_SHADER_VISIBILITY_VERTEX);

		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // 只有输入汇编程序阶段才需要访问常量缓冲区。
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

	std::array<concurrency::task<std::vector<byte>>, 2> VsPsTasks{ 
		resourceResolver->ResovleShader(TileLayerVSName), resourceResolver->ResovleShader(TileLayerPSName) };
	co_await concurrency::when_all(VsPsTasks.begin(), VsPsTasks.end());
	auto vs = VsPsTasks[0].get();
	auto ps = VsPsTasks[1].get();

	static const D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC state = {};
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
}
