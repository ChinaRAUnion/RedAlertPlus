// 通过像素着色器传递的每个像素的颜色数据。
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

// (内插)颜色数据的传递函数。
float4 main(PixelShaderInput input) : SV_TARGET
{
	//float4 color = g_texture.Sample(g_sampler, input.uv);
	//clip(color.a - 1.f);
	return float4(1.f, 0, 0, 1.f);
}
