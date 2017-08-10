#include "Perspective.hlsli"

// Simple shader to do vertex processing on the GPU.
PixelShaderInput main(VERTEX_3D input)
{
	PixelShaderInput output;
	float4 pos = float4(input.pos.xyz, 1.0f);

	// Transform the vertex position into projected space.
	output.pos = MultiPerspective(pos);

	// Pass the color through without modification.
	if (SkyboxToggle){
		output.uv = input.pos.xyz;
	}
	else {
		output.uv.xy = input.uv;
	}
	return output;
}
