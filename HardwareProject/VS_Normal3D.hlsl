#include "Perspective.hlsli"

// Simple shader to do vertex processing on the GPU.
LightingShaderInput main(VERTEX_OBJMODEL input)
{
	LightingShaderInput output;
	float4 posT = float4(input.pos, 1.0f);
	float4 normT = float4(input.norm, 1.0f);

	// Transform the vertex position into projected space.
	output.pos = posT;
	output.uv = input.uv;
	output.norm = mul(normT, model);

	return output;
}
