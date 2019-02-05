#include "Common.inl"

Texture2D shaderTextures[2] : register(t0);
Texture2D directionalShadowMap : register(t2);
SamplerState sampleTypeWrap : register(s0);
SamplerComparisonState sampleTypeShadows : register(s1);

cbuffer LightBuffer : register(b0)
{
	float4 LightColor;
	float4 LightDirectionAndIntensity;
	float4 BiasOffsetRes;
	matrix ShadowSpace;
};

struct INPUT_PIXEL
{
	float4 Pos : SV_POSITION;
	float3 PosWS : POSITION;
	float2 Tex : TEXCOORD0;
	float3 NormalVS : NORMAL0;
	float3 NormalWS : NORMAL1;
	float3 TangentVS : TANGENT;
	float3 BinormalVS : BINORMAL;
};



float4 main(INPUT_PIXEL pIn) : SV_TARGET
{
	float4 textureColor;
	float3 normalMap;
	float3 normal;
	float3 lightDir;
	float lightRatio;
	float shadowRatio = 1.0f;
	float4 color;

	textureColor = shaderTextures[0].Sample(sampleTypeWrap, pIn.Tex);
	normalMap = shaderTextures[1].Sample(sampleTypeWrap, pIn.Tex).wyz * 2.f - 1.f;
	normalMap.z = sqrt(1 - normalMap.x*normalMap.x - normalMap.y * normalMap.y);

	normal = (normalMap.x * pIn.TangentVS) + (normalMap.y * pIn.BinormalVS) + (normalMap.z * pIn.NormalVS);
	//normal = pIn.Normal;

	normal = normalize(normal);

	lightDir = LightDirectionAndIntensity.xyz * -1.f;
	lightDir = normalize(lightDir);

	lightRatio = saturate(dot(normal, lightDir));
	float txlSize = 1.0f / (BiasOffsetRes.z);

	float cosAngle = saturate(1.f - dot(pIn.NormalVS, lightDir));
	float3 normalOffset = (BiasOffsetRes.y * cosAngle) * pIn.NormalWS;

	float4 lightSpacePos = mul(float4(pIn.PosWS + normalOffset, 1), ShadowSpace);
	float3 lspProj = lightSpacePos.xyz / lightSpacePos.w;
	// Screencoords to NDC
	float2 shadowCoords;
	shadowCoords.x = lspProj.x / 2.0f + 0.5f;
	shadowCoords.y = -lspProj.y / 2.0f + 0.5f;
	//Check if within shadowmap bounds
	if((saturate(shadowCoords.x) == shadowCoords.x) && (saturate(shadowCoords.y) == shadowCoords.y))
	{
		//Get depth w/ bias
		float z = lspProj.z - BiasOffsetRes.x;

		shadowRatio = PCFBlur(shadowCoords, 2, z, directionalShadowMap, sampleTypeShadows, txlSize);
	}

	color = saturate(LightColor * LightDirectionAndIntensity.w * lightRatio * shadowRatio);
	color = color * textureColor + textureColor * 0.6f;

	//return float4(1.0f, 1.0f, 1.0f, 1.0f);
	return color;
}