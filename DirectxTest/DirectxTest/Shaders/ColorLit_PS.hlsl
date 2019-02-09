#include "PSIncludes.hlsl"

Texture2D directionalShadowMap : register(t6);
SamplerState sampleTypeWrap : register(s0);
SamplerComparisonState sampleTypeShadows : register(s1);

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

    textureColor = float3(0.5f, 0.5f, 0.5f);

	normal = pIn.NormalWS;
	normal = normalize(normal);

    float3 viewWS = pIn.PosWS - _EyePosition;;
    viewWS = normalize(viewWS);

	//return float4(1.0f, 1.0f, 1.0f, 1.0f);
    return BlinnPhong(surface, normal, textureColor, pIn.NormalWS, pIn.PosWS, viewWS, lightInfo, directionalShadowMap, sampleTypeShadows);
}