#pragma once

#include "PSIncludes.hlsl"

Texture2D diffuseMap : register(t0);
SamplerState sampleTypeWrap : register(s0);


struct INPUT_PIXEL
{
    float4 Pos : SV_POSITION;
    float3 PosWS : POSITION;
    float2 Tex : TEXCOORD0;
    float3 NormalWS : NORMAL;
    float3 TangentWS : TANGENT;
    float3 BinormalWS : BINORMAL;
};

float4 main(INPUT_PIXEL pIn) : SV_TARGET
{
	
    float2 uv = pIn.PosWS.xy / 2.0f + sin(_Time * 0.3f)*3.f;
    float3 color = saturate(diffuseMap.Sample(sampleTypeWrap, uv).xyz*3.f);

    return float4(color, 1.0f);

}