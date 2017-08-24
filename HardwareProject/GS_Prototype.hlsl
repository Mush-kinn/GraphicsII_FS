//struct GSOutput
//{
//	float4 pos : SV_POSITION;
//};

#include "Perspective.hlsli"
[maxvertexcount(6)]
void main(triangle PixelShaderInput input[3], inout TriangleStream< PixelShaderInput > output)
{
	for (uint i = 0; i < 3; i++){
		PixelShaderInput element;
		element.pos = MultiPerspective(input[i].pos, 0);
		element.uv = input[i].uv;
		output.Append(element);
	}
	output.RestartStrip();
	
}