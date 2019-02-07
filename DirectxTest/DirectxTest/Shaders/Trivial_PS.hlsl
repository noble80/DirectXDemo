#include "Common.hlsl"

Texture2D shaderTextures[2] : register(t0);
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
    float2 Tex : TEXCOORD0;
    float3 NormalWS : NORMAL;	
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
};

float4 main(INPUT_PIXEL pIn) : SV_TARGET
{
	float3 textureColor;
	float3 normalMap;
	float3 normal;

	textureColor = shaderTextures[0].Sample(sampleTypeWrap, pIn.Tex).xyz;
	normalMap = shaderTextures[1].Sample(sampleTypeWrap, pIn.Tex).wyz * 2.f - 1.f;
	normalMap.z = sqrt(1 - normalMap.x*normalMap.x - normalMap.y * normalMap.y);

    normal = normalMap.x * pIn.tangent + normalMap.y * pIn.binormal + normalMap.z * pIn.NormalWS;
	//normal = pIn.NormalWS;
	normal = normalize(normal);

    return bdrf(normal, pIn.NormalWS, pIn.PosWS, textureColor, 0.6f, lightInfo, directionalShadowMap, sampleTypeShadows);
}