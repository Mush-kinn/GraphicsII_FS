//struct GSOutput
//{
//	float4 pos : SV_POSITION;
//};

#include "Perspective.hlsli"
#define NUM_PORTS 2
[maxvertexcount(6)]
void main(triangle PixelShaderInput input[3], inout TriangleStream< GS_PixelShaderInput > output )
{
	for (uint j = 0; j < NUM_PORTS; ++j){
		for (uint i = 0; i < 3; i++){
			GS_PixelShaderInput element;
			element.pos = MultiPerspective(input[i].pos, j);
			element.uv = input[i].uv;
			element.outIndex = j;
			output.Append(element);
		}
		output.RestartStrip();
	}
}