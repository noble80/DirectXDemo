#pragma once
#include "Common.hlsl"

cbuffer SceneInfoBuffer : register(b5)
{
    float3 _EyePosition;
    float _Time;
};

cbuffer LightInfoBuffer : register(b6)
{
    LightInfo lightInfo;
};


cbuffer CSurfaceProperties : register(b4)
{
    float3 _diffuseColor;
    float _specularIntensity;
    float3 _emissiveColor;
    float _glossinessRoughness;
    float _normalIntensity;
    float _ambientIntensity;
    int _textureFlags;
    float padding;
};

float3 SampleShadows(float3 worldPos, float linearDepth)
{
    float txlSize = 1.0f / (lightInfo.directionalShadowInfo.resolution);
    int currCascade = -1;

    float shadowRatio = 1.0f;

    for (int i = CASCADES_MAX - 1; i >= 0; --i)
    {
        if (linearDepth <= lightInfo.directionalShadowInfo.cascades[i].cascadeSplit)
        {
            currCascade = i;
        }
    }

    //return (float) currCascade / (CASCADES_MAX - 1);

    if (currCascade >= 0)
    {
		// Screencoords to NDC
        float4 lightSpacePos = mul(float4(worldPos, 1), lightInfo.directionalShadowInfo.cascades[currCascade].lightSpace);
        float3 lspProj = lightSpacePos.xyz / lightSpacePos.w;
        float2 shadowCoords;
        shadowCoords.x = lspProj.x / 2.0f + 0.5f;
        shadowCoords.y = -lspProj.y / 2.0f + 0.5f;

        shadowCoords = shadowCoords;
            //Get depth w/ bias
        float z = lspProj.z - lightInfo.directionalShadowInfo.bias;
        shadowRatio = PCFBlur(shadowCoords.xy, currCascade, 25, z, txlSize, worldPos);

        const float BlendThreshold = 0.1f;

        float nextSplit = lightInfo.directionalShadowInfo.cascades[currCascade].cascadeSplit;
        float splitSize = currCascade == 0 ? nextSplit : nextSplit - lightInfo.directionalShadowInfo.cascades[currCascade - 1].cascadeSplit;
        float blendRatio = (nextSplit - linearDepth) / splitSize;
        float alpha = smoothstep(0.0f, BlendThreshold, blendRatio);

        if (alpha > 0.0f)
        {
            lightSpacePos = mul(float4(worldPos, 1), lightInfo.directionalShadowInfo.cascades[currCascade + 1].lightSpace);
            lspProj = lightSpacePos.xyz / lightSpacePos.w;
            shadowCoords.x = lspProj.x / 2.0f + 0.5f;
            shadowCoords.y = -lspProj.y / 2.0f + 0.5f;

            z = lspProj.z - lightInfo.directionalShadowInfo.bias;
            float nextShadowRatio = PCFBlur(shadowCoords.xy, currCascade + 1, 25, z, txlSize, worldPos);
            shadowRatio = lerp(nextShadowRatio, shadowRatio, alpha);
        }
        return shadowRatio;
    }
    return 1.0f;
}