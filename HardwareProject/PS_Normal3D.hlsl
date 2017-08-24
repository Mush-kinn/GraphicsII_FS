#include "Perspective.hlsli"

Texture2D testMap : register(t0);
Texture2D skin : register(t1);

SamplerState s : register(s0);

// A pass-through function for the (interpolated) color data.
float4 main(GS_LightingShaderInput input) : SV_TARGET
{
	float4 difuseL;

	input.norm = normalize(input.norm);
	difuseL = skin.Sample(s, input.uv);

	float3 finalColor;

	finalColor = difuseL * ambient;
	finalColor += saturate(dot(direction, input.norm) * difuse * difuseL);

	return float4(finalColor, difuseL.a);

}