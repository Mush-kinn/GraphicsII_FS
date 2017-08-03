#include "Perspective.hlsli"

// Simple shader to do vertex processing on the GPU.
LightingShaderInput main(VERTEX_3D_NORM input)
{
	LightingShaderInput output;
	float4 pos = float4(input.pos, 1.0f);

		// Transform the vertex position into projected space.
		output.pos = MultiPerspective(pos);

	// Pass the color through without modification.
	output.uv = input.uv;
	input.norm.x = 0;
	output.norm = mul(input.norm, model);

	return output;
}
