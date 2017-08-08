
#include "SharedDefines.h"

float4 MultiPerspective(float4 pos){

	pos = mul(pos, model);
	pos = mul(pos, view);
	pos = mul(pos, projection);
	return pos;
}

struct PixelShaderInput{
	float4 pos : SV_POSITION;
	float3 uv : TEXCOORD;
};

struct LightingShaderInput{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
	float3 norm : NORMAL;
};