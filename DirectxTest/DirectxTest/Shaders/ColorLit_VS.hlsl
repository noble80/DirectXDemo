#include "VSIncludes.hlsl"

struct INPUT_VERTEX
{
	float3 Pos : POSITION;
	float3 Normal : NORMAL;
    float2 Tex : TEXCOORD0;
};

struct OUTPUT_VERTEX
{
	float4 Pos : SV_POSITION;
	float3 PosWS : POSITION;
	float3 NormalWS : NORMAL;
    float2 Tex : TEXCOORD0;
};


OUTPUT_VERTEX main( INPUT_VERTEX vIn )
{
	OUTPUT_VERTEX output = (OUTPUT_VERTEX)0;
	const float4 Pos = float4(vIn.Pos, 1);
	output.PosWS = mul(Pos, World).xyz;

    output.Tex = vIn.Tex;

    output.NormalWS = mul(float4(vIn.Normal, 0), Normal).xyz;
	output.NormalWS = normalize(output.NormalWS);

	output.Pos = mul(Pos, WorldViewProjection);

	return output;
}