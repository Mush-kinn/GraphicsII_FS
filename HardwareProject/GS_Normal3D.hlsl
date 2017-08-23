#include "Perspective.hlsli"
#define NUM_PORTS 2
[maxvertexcount(6)]
void main(triangle LightingShaderInput input[3], inout TriangleStream< GS_LightingShaderInput > output)
{
	for (uint j = 0; j < NUM_PORTS; ++j){
		for (uint i = 0; i < 3; i++){
			GS_LightingShaderInput element;
			element.pos = MultiPerspective(input[i].pos, j);
			element.uv = input[i].uv;
			element.norm.xyz = mul(input[i].norm, model).xyz;
			element.outIndex = j;
			output.Append(element);
		}
		output.RestartStrip();
	}
}