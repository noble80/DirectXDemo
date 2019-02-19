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
    float4 Pos = float4(vIn.Pos, 1);

    float2 uv = vIn.Tex;
    float O = radians(-22.5f);
    float2 rotUV = float2(uv.x * cos(O) - uv.y * sin(O), uv.x * cos(O) + uv.y * sin(O));
    float z = 0.1f * sin(15.f * rotUV.x + 12.f * _Time);
    //float z = sin(time)*0.5 + 0.5;

    float waveDis = z * (1.f - vIn.Tex.x);
    Pos.z += waveDis;
    output.Pos = mul(Pos, WorldViewProjection);
    output.PosWS = mul(Pos, World).xyz;

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