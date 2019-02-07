#include "Common.hlsl"

Texture2D AOTexture : register(t0);

Texture2D directionalShadowMap : register(t6);
SamplerState sampleTypeWrap : register(s0);
SamplerComparisonState sampleTypeShadows : register(s1);

cbuffer LightBuffer : register(b0)
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

    float AOSample = AOTexture.Sample(sampleTypeWrap, pIn.Tex);

    textureColor = float3(0.5f, 0.5f, 0.5f);

	normal = pIn.NormalWS;
	normal = normalize(normal);

	//return float4(1.0f, 1.0f, 1.0f, 1.0f);
    return bdrf(normal, pIn.NormalWS, pIn.PosWS, textureColor, saturate(0.6f * AOSample), lightInfo, directionalShadowMap, sampleTypeShadows);
}