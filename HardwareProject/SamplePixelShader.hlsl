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

	else{
		//temp = testMap.Sample(s, input.uv.xy);

		float4 difuseL;

		float3 lightDir = mul(input.pos, boxM);
		 lightDir = normalize(direction - lightDir);
		float lightRatio = clamp(dot(lightDir, float3(0, 1, 0)), 0, 1);

		difuseL = testMap.Sample(s, input.uv);
		float4 lightColor = Skybox.Sample(s, lightDir);

		float3 finalColor;
		//finalColor = difuseL * ambient;
		finalColor = 1 * lightColor * difuseL ;

		return float4(finalColor, difuseL.a);
	}

	return temp;
}
