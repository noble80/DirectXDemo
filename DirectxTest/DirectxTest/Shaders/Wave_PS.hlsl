#include "Common.hlsl"

Texture2D directionalShadowMap : register(t6);
SamplerState sampleTypeWrap : register(s0);
SamplerComparisonState sampleTypeShadows : register(s1);

cbuffer LightInfoBuffer : register(b6)
{
    LightInfo lightInfo;
};

struct INPUT_PIXEL
{
	float4 Pos : SV_POSITION;
	float3 PosWS : POSITION;
	float3 NormalWS : NORMAL;
    float2 Tex : TEXCOORD0;
};

float4 main(INPUT_PIXEL pIn) : SV_TARGET
{
	float3 textureColor;
	float3 normal;

    textureColor = float3(0.5f, 0.6f, 0.8f);

	normal = pIn.NormalWS;
	normal = normalize(normal);

	//return float4(1.0f, 1.0f, 1.0f, 1.0f);
    return bdrf(normal, pIn.NormalWS, pIn.PosWS, textureColor, 0.6f, lightInfo, directionalShadowMap, sampleTypeShadows);
}