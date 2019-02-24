#include "VSIncludes.hlsl"

struct INPUT_VERTEX
{
    float3 Pos : POSITION;
};

struct OUTPUT_VERTEX
{
    float4 Pos : SV_POSITION;
    float3 PosLS : POSITION;
};


OUTPUT_VERTEX main(INPUT_VERTEX vIn)
{
    OUTPUT_VERTEX output = (OUTPUT_VERTEX) 0;
    const float4 Pos = float4(vIn.Pos, 1);
    output.Pos = mul(Pos, World);
    output.Pos = mul(output.Pos, ViewProjection).xyww;
    output.PosLS = vIn.Pos.xyz;

    return output;
}

