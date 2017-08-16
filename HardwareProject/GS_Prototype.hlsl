//struct GSOutput
//{
//	float4 pos : SV_POSITION;
//};

#include "Perspective.hlsli"

[maxvertexcount(3)]
void main(triangle PixelShaderInput input[3], inout TriangleStream< PixelShaderInput > output )
{
	for (uint i = 0; i < 3; i++)
	{
		PixelShaderInput element;
		element.pos = input[i].pos;
		element.uv = input[i].uv;
		output.Append(element);
	}
}