#include "BlinnPhong_Include.hlsl"

Texture2D diffuseMap : register(t0);
Texture2D detailsMap : register(t1);
Texture2D normalMap : register(t2);
TextureCube ReflectionMap : register(t3);


struct INPUT_PIXEL
{
    float4 Pos : SV_POSITION;
    float3 PosWS : POSITION;
    float2 Tex : TEXCOORD0;
    float3 NormalWS : NORMAL;
    float3 TangentWS : TANGENT;
    float3 BinormalWS : BINORMAL;
    float linearDepth : DEPTH;
};

float4 main(INPUT_PIXEL pIn) : SV_TARGET
{
    float3 viewWS = normalize(pIn.PosWS - _EyePosition);
    float reflectionLevel;
    float3 reflection;
    float3 normalSample;

    SurfaceBlinnPhong surface;
    surface.diffuseColor = _diffuseColor;
    surface.specularIntensity = _specularIntensity;
    surface.glossiness = _glossinessRoughness;

    surface.ambient = _ambientIntensity;
    surface.emissiveColor = _emissiveColor;

    if (HasDiffuseTexture(_textureFlags))
    {
        float3 diffuse = diffuseMap.Sample(sampleTypeWrap, pIn.Tex).xyz;
        surface.diffuseColor *= diffuse;
    }

    if (HasSpecularTexture(_textureFlags))
    {
        surface.specularIntensity *= detailsMap.Sample(sampleTypeWrap, pIn.Tex).x;
    }

    if (HasGlossinessTexture(_textureFlags))
    {
        surface.glossiness *= detailsMap.Sample(sampleTypeWrap, pIn.Tex).y;
    }

    if (HasAOTexture(_textureFlags))
    {
        surface.ambient *= detailsMap.Sample(sampleTypeWrap, pIn.Tex).z;
    }

    if (HasEmissiveMask(_textureFlags))
    {
        surface.emissiveColor *= diffuseMap.Sample(sampleTypeWrap, pIn.Tex).w;
    }

    if (HasNormalTexture(_textureFlags))
    {
        normalSample = normalMap.Sample(sampleTypeWrap, pIn.Tex).wyz * 2.f - 1.f;
        normalSample.z = sqrt(1 - normalSample.x * normalSample.x - normalSample.y * normalSample.y);
        surface.normal = pIn.TangentWS * normalSample.x + pIn.BinormalWS * normalSample.y + pIn.NormalWS * normalSample.z;
    }
    else
    {
        surface.normal = pIn.NormalWS;
    }
    surface.normal = normalize(surface.normal);

    float3 reflectionVector = reflect(viewWS, surface.normal);


    float3 positionWS = pIn.PosWS;
    float3 color = surface.ambient*lightInfo.ambientColor*surface.diffuseColor + surface.emissiveColor;

    if (HasReflections(_textureFlags))
    {
        reflectionLevel = saturate(1.f - surface.glossiness) * 10.f;
        reflection = surface.specularIntensity*ReflectionMap.SampleLevel(sampleTypeClamp, reflectionVector, reflectionLevel).xyz;
        color += reflection;
    }

    for (int i = 0; i < lightInfo.pointLightCount; i++)
    {
        float radius = lightInfo.pointLights[i].radius;
        float3 lightPos = lightInfo.pointLights[i].position;
        float3 dir = normalize(lightPos - positionWS);
        float dist = length(lightPos - positionWS);
        float attenuation = saturate(1.0f - dist * dist / (radius * radius));
        attenuation *= attenuation;

        color += lightInfo.pointLights[i].color * attenuation * BlinnPhong(surface, dir, viewWS);

    }

    for (i = 0; i < lightInfo.spotLightCount; i++)
    {
        float radius = lightInfo.spotLights[i].radius;
        float outerCone = lightInfo.spotLights[i].outerCone;
        float innerCone = lightInfo.spotLights[i].innerCone;
        float3 lightPos = lightInfo.spotLights[i].position;
        float3 dir = normalize(lightPos - positionWS);
        float dist = length(lightPos - positionWS);

        float pAttenuation = saturate(1.0f - dist * dist / (radius * radius));
        pAttenuation *= pAttenuation;

        float sAttenuation = saturate((dot(dir, -lightInfo.spotLights[i].direction) - outerCone) / (innerCone - outerCone));
        sAttenuation *= sAttenuation;

        color += lightInfo.spotLights[i].color * sAttenuation * pAttenuation * BlinnPhong(surface, dir, viewWS);
    }

	{
        float lightRatio = saturate(dot(surface.normal, -lightInfo.directionalLight.direction));
        float3 normalOffset = pIn.NormalWS * (lightInfo.directionalShadowInfo.normalOffset * (1.f - lightRatio));

        float3 shadowRatio = SampleShadows(positionWS + normalOffset, pIn.linearDepth);

        color += lightInfo.directionalLight.color * shadowRatio * BlinnPhong(surface, -lightInfo.directionalLight.direction, viewWS);
    }

    //color = pow(color, 1.f / 2.2f);
    return float4(color, 1.f);
}