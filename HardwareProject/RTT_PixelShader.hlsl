#include "Perspective.hlsli"

Texture2D testMap : register(t0);
TextureCube Skybox : register(t1);

SamplerState s : register(s0);

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{
	float4 temp;
	if (SkyboxToggle){
		temp = Skybox.Sample(s, input.uv);
	}
	else {
		temp = testMap.Sample(s, input.uv.xy);
	}
	return temp;
}
