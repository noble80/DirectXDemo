#include "PBR_Include.hlsl"

cbuffer CTerrainBuffer : register(b7)
{
    float2 _TextureTiling;
    float _TxlSizeU;
    float _TxlSizeV;
    float _PatchSpacing;
    float _TesselationFactor;
    float _TesselationProj;
};

Texture2D diffuseMap : register(t0);
Texture2D normalMap : register(t1);
TextureCube IBLDiffuse : register(t3);
TextureCube IBLSpecular : register(t4);
Texture2D IBLIntegration : register(t5);
Texture2D HeightMap : register(t7);
Texture2D detailColorA : register(t8);
Texture2D detailNormalA : register(t9);
Texture2D detailColorB : register(t10);
Texture2D detailNormalB : register(t11);
Texture2D terrianMask : register(t12);


struct DomainOutput
{
    float4 Pos : SV_POSITION;
	// TODO: change/add other stuff
    float3 PosWS : POSITION;
    float2 Tex : TEXCOORD0;
    float2 Tex2 : TEXCOORD1;
    float linearDepth : DEPTH;
};

float4 main(DomainOutput pIn) : SV_TARGET
{
    float3 tangentWS = float3(1.0f, 0.f, 0.f);
    float3 binormalWS = float3(0.f, 0.f, 1.0f);
    float3 normalWS = cross(binormalWS, tangentWS);

    float3 viewWS = normalize(pIn.PosWS - _EyePosition);
    float3 normalSample;

    SurfacePBR surface;
    surface.diffuseColor = 1.0f;
    surface.metallic = 0.f;
    surface.roughness = 0.8f;

    surface.ambient = 1.0f;
    surface.emissiveColor = _emissiveColor;

    float mask = saturate(1.f - terrianMask.Sample(sampleTypeClamp, pIn.Tex).r);

    float4 diffuse = diffuseMap.Sample(sampleTypeWrap, pIn.Tex);

    float3 colorA = detailColorA.Sample(sampleTypeWrap, pIn.Tex2).rgb;
    float3 colorB = detailColorB.Sample(sampleTypeWrap, pIn.Tex2).rgb;
    float3 colorBlend = lerp(colorA, colorB, mask);

    surface.diffuseColor *= diffuse.xyz * colorBlend;
    surface.diffuseColor = saturate(surface.diffuseColor);

    //Remapping roughness to prevent errors on normal distribution
    surface.roughness = remap(surface.roughness, 0.0f, 1.0f, 0.08f, 1.0f);

    float2 normalA = detailNormalA.Sample(sampleTypeWrap, pIn.Tex2*3).rg*2.f - 1.f;
    float2 normalB = detailNormalB.Sample(sampleTypeWrap, pIn.Tex2*2).rg*2.f - 1.f;
    normalB *= 2.f;
    float2 normalBlend = lerp(normalA, normalB, mask);
 
    normalSample = normalMap.Sample(sampleTypeWrap, pIn.Tex).xyz * 2.f - 1.f;
    normalSample.z = sqrt(1 - normalSample.x * normalSample.x - normalSample.y * normalSample.y);
    normalSample.rg += normalBlend;
    surface.normal = 2 * tangentWS * normalSample.x + 2 * binormalWS * normalSample.y + normalWS * normalSample.z;
    surface.normal = normalize(surface.normal);

    float3 reflectionVector = reflect(viewWS, surface.normal);


    float3 positionWS = pIn.PosWS;
    float3 color = 0.0f;

    if (HasReflections(_textureFlags))
    {
        
    }

    // Non metals use a constant for this value
    float3 specColor = 0.04f;
    // Lerp between diffuse color and constant based on metallic. Ideally metallic should be either 1 or 0
    specColor = lerp(specColor, surface.diffuseColor, surface.metallic);

    for (int i = 0; i < lightInfo.pointLightCount; i++)
    {
        float radius = lightInfo.pointLights[i].radius;
        float3 lightPos = lightInfo.pointLights[i].position;
        float3 dir = normalize(lightPos - positionWS);
        float dist = length(lightPos - positionWS);
        float attenuation = saturate(1.0f - dist * dist / (radius * radius));
        attenuation *= attenuation;
        float3 radiance = lightInfo.pointLights[i].color * attenuation;

        color += PBR(surface, dir, viewWS, specColor) * radiance;

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

        float3 radiance = lightInfo.spotLights[i].color * sAttenuation * pAttenuation;

        color += PBR(surface, dir, viewWS, specColor) * radiance;
    }

	{
        float lightRatio = saturate(dot(surface.normal, -lightInfo.directionalLight.direction));
        float3 normalOffset = normalWS * (lightInfo.directionalShadowInfo.normalOffset * (1.f - lightRatio));

        float3 shadowRatio = SampleShadows(positionWS + normalOffset, pIn.linearDepth) * clamp(diffuse.a, 0.0f, 1.0f);

        float3 radiance = lightInfo.directionalLight.color * shadowRatio;

        color += radiance * PBR(surface, -lightInfo.directionalLight.direction, viewWS, specColor);
    }

    //Environment mapping
    {
        surface.ambient *= lightInfo.ambientColor;
        float3 N = surface.normal;
        float3 V = -viewWS;

        float NdotV = max(dot(N, V), 0.f);
        float3 diffuse = IBLDiffuse.Sample(sampleTypeWrap, N).rgb;
        float3 specular = IBLSpecular.SampleLevel(sampleTypeWrap, reflectionVector, surface.roughness * 10.f).rgb;
        float2 integration = IBLIntegration.Sample(sampleTypeNearest, float2(NdotV, 1.f - surface.roughness)).rg;
        color += IBL(surface, viewWS, specColor, diffuse, specular, integration) * surface.ambient;
    }

    // Reinhard operator. Supposedly preserves HDR values better
    //color = color / (color + 1.f);
    //color = pow(color, 1.f / 2.2f);
    return float4(color, 1.f);
}