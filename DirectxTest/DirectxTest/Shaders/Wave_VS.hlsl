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
};


OUTPUT_VERTEX main(INPUT_VERTEX vIn)
{
    OUTPUT_VERTEX output = (OUTPUT_VERTEX) 0;
    float4 Pos = float4(vIn.Pos, 1);
    float alpha = (sin(_Time * 3.f) + 1) / 2.f;
    alpha *= (sin(Pos.y * 10.f) + 1) / 2.f;
    alpha *= (sin(Pos.z * 10.f) + 1) / 2.f;
    alpha *= (sin(Pos.x * 10.f) + 1) / 2.f;

    float waveDis = lerp(-0.5f, 0.5f, alpha);
    Pos += float4(waveDis * vIn.Normal, 0.f);
    output.Pos = mul(Pos, WorldViewProjection);
    output.PosWS = mul(Pos, World).xyz;

    output.Tex = vIn.Tex;

    output.TangentWS = mul(float4(vIn.Tangent, 0), Normal).xyz;
    output.TangentWS = normalize(output.TangentWS);
	
    output.BinormalWS = mul(float4(vIn.Binormal, 0), Normal).xyz;
    output.BinormalWS = normalize(output.BinormalWS);
	
    output.NormalWS = mul(float4(vIn.Normal, 0), Normal).xyz;
    output.NormalWS = normalize(output.NormalWS);
    
    return output;
}