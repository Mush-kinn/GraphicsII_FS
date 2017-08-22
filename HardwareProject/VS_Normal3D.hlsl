#include "Perspective.hlsli"

// Simple shader to do vertex processing on the GPU.
LightingShaderInput main(VERTEX_3D_NORM input)
{
	LightingShaderInput output;
	float4 pos = float4(input.pos, 1.0f);
	float4 norm = float4(input.norm, 1.0f);

	// Transform the vertex position into projected space.
	output.pos = MultiPerspective(pos,0);
	output.uv = input.uv;
	output.norm.xyz = mul(norm, model).xyz;

	return output;
}
