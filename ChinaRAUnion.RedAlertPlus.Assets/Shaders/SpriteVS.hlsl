// 存储用于构成几何图形的三个基本列优先矩阵的常量缓冲区。
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};

// 用作顶点着色器输入的每个顶点的数据。
struct VertexShaderInput
{
	float3 pos : POSITION;
	uint id : Id;
	uint remapable : Remapable;
};

// 通过像素着色器传递的每个像素的颜色数据。
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

// 用于在 GPU 上执行顶点处理的简单着色器。
PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	float4 pos = float4(input.pos, 1.0f);

	// 将顶点位置转换为投影空间。
	//pos = mul(pos, model);
	pos = mul(pos, view);
	pos = mul(pos, projection);
	output.pos = pos;

	output.uv = float2(1.0f, 1.0f);

	return output;
}
