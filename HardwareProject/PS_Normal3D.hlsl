#include "Perspective.hlsli"

Texture2D testMap : register(t0);
Texture2D skin : register(t1);

SamplerState s : register(s0);

// A pass-through function for the (interpolated) color data.
float4 main(LightingShaderInput input) : SV_TARGET
{
	float4 temp;
	temp.grab = testMap.Sample(s, input.uv);





	return temp.grab;
}