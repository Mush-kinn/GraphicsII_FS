
#include "SharedDefines.h"

float4 MultiPerspective(float4 pos, uint indx){

	pos = mul(pos, model);
	pos = mul(pos, view[indx]);
	pos = mul(pos, projection[indx]);
	return pos;
}

struct PixelShaderInput{
	float4 pos : SV_POSITION;
	float3 uv : TEXCOORD;
};

struct GS_PixelShaderInput{
	float4 pos : SV_POSITION;
	float3 uv : TEXCOORD;
	uint outIndex : SV_ViewportArrayIndex;
};

struct LightingShaderInput{
	float4 pos : SV_POSITION;
	float3 uv : TEXCOORD;
	float3 norm : NORMAL;
};

struct GS_LightingShaderInput{
	float4 pos : SV_POSITION;
	float3 uv : TEXCOORD;
	float3 norm : NORMAL;
	uint outIndex : SV_ViewportArrayIndex;
};