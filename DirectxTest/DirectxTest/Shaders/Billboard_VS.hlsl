#include "VSIncludes.hlsl"

struct INPUT_VERTEX
{
    float3 Pos : POSITION;
    float2 Tex : TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
};

struct OUTPUT_VERTEX
{
    float4 Pos : SV_POSITION;
    float3 PosWS : POSITION;
    float2 Tex : TEXCOORD0;
    float3 NormalWS : NORMAL;
    float3 TangentWS : TANGENT;
    float3 BinormalWS : BINORMAL;
    float linearDepth : DEPTH;
};


OUTPUT_VERTEX main(INPUT_VERTEX vIn)
{
    OUTPUT_VERTEX output = (OUTPUT_VERTEX) 0;

    matrix bWorldView = WorldView;

    bWorldView[0][0] = 90.0;
    bWorldView[0][1] = 0.0;
    bWorldView[0][2] = 0.0;
                 
    //bWorldView[1][0] = 0.0;
    //bWorldView[1][1] = 90.0;
    //bWorldView[1][2] = 0.0;
                 
    bWorldView[2][0] = 0.0;
    bWorldView[2][1] = 0.0;
    bWorldView[2][2] = 1.0;

    const float4 Pos = float4(vIn.Pos, 1);
    output.PosWS = mul(Pos, World).xyz;
    output.Pos = mul(Pos, bWorldView);
    output.Pos = mul(output.Pos, Projection);
    output.Pos.z = min(output.Pos.z, output.Pos.w);

    output.Tex = vIn.Tex;

    output.TangentWS = mul(float4(vIn.Tangent, 0), Normal).xyz;
    output.TangentWS = normalize(output.TangentWS);
	
    output.BinormalWS = mul(float4(vIn.Binormal, 0), Normal).xyz;
    output.BinormalWS = normalize(output.BinormalWS);
	
    output.NormalWS = mul(float4(vIn.Normal, 0), Normal).xyz;
    output.NormalWS = normalize(output.NormalWS);

    output.linearDepth = output.Pos.w;

    return output;
}