#include "Perspective.hlsli"

// Simple shader to do vertex processing on the GPU.
PixelShaderInput main(VERTEX_3D input)
{
	PixelShaderInput output;

	// Transform the vertex position into projected space.
	output.pos = float4(input.pos.xyz, 1.0f);

	// Pass the color through without modification.
	if (SkyboxToggle){
		output.uv = input.pos.xyz;
	}
	else {
		output.uv.xy = input.uv;
	}
	return output;
}
