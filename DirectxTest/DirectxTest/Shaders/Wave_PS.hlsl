#include "Common.hlsl"

Texture2D directionalShadowMap : register(t6);
SamplerState sampleTypeWrap : register(s0);
SamplerComparisonState sampleTypeShadows : register(s1);

cbuffer SceneInfoBuffer : register(b5)
{
    float3 _EyePosition;
    float _Time;
};

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

    float3 viewWS = _EyePosition + pIn.PosWS;
    viewWS = normalize(viewWS);

	//return float4(1.0f, 1.0f, 1.0f, 1.0f);
    return BlinnPhong(normal, pIn.NormalWS, pIn.PosWS, viewWS, textureColor, 0.6f, lightInfo, directionalShadowMap, sampleTypeShadows);
}