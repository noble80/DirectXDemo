#include "PBR_Include.hlsl"

Texture2D diffuseMap : register(t0);
Texture2D detailsMap : register(t1);
Texture2D normalMap : register(t2);
TextureCube IBLDiffuse : register(t3);
TextureCube IBLSpecular : register(t4);
Texture2D IBLIntegration : register(t5);
Texture2D directionalShadowMap : register(t6);

SamplerState sampleTypeWrap : register(s0);
SamplerComparisonState sampleTypeShadows : register(s1);
SamplerState sampleTypeClamp : register(s2);
SamplerState sampleTypeNearest : register(s3);

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
    float3 viewWS = normalize(pIn.PosWS - _EyePosition);
    float3 normalSample;

    SurfacePBR surface;
    surface.diffuseColor = _diffuseColor;
    surface.metallic = _specularIntensity;
    surface.roughness = _glossinessRoughness;

    surface.ambient = _ambientIntensity;
    surface.emissiveColor = _emissiveColor;

    if (HasDiffuseTexture(_textureFlags))
    {
        float3 diffuse = diffuseMap.Sample(sampleTypeWrap, pIn.Tex).xyz;
        surface.diffuseColor *= diffuse;
    }

    if (HasSpecularTexture(_textureFlags))
    {
        surface.metallic *= detailsMap.Sample(sampleTypeWrap, pIn.Tex).x;
    }

    if (HasGlossinessTexture(_textureFlags))
    {
        surface.roughness *= detailsMap.Sample(sampleTypeWrap, pIn.Tex).y;
    }
    //Remapping roughness to prevent errors on normal distribution
    surface.roughness = remap(surface.roughness, 0.0f, 1.0f, 0.05f, 1.0f);

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
    float3 color = surface.emissiveColor; //surface.ambient * lightInfo.ambientColor * surface.diffuseColor +

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
        float shadowRatio = 1.0f;
        float txlSize = 1.0f / (lightInfo.directionalShadowInfo.resolution);

        float lightRatio = saturate(dot(surface.normal, -lightInfo.directionalLight.direction));
        float3 normalOffset = pIn.NormalWS * (lightInfo.directionalShadowInfo.normalOffset * (1.f - lightRatio));

        float4 lightSpacePos = mul(float4(positionWS + normalOffset, 1), lightInfo.directionalShadowInfo.viewProj);
        float3 lspProj = lightSpacePos.xyz / lightSpacePos.w;
		// Screencoords to NDC
        float2 shadowCoords;
        shadowCoords.x = lspProj.x / 2.0f + 0.5f;
        shadowCoords.y = -lspProj.y / 2.0f + 0.5f;
		//Check if within shadowmap bounds
        if ((saturate(shadowCoords.x) == shadowCoords.x) && (saturate(shadowCoords.y) == shadowCoords.y))
        {
			//Get depth w/ bias
            float z = lspProj.z - lightInfo.directionalShadowInfo.bias;

            shadowRatio = PCFBlur(shadowCoords, 2, z, directionalShadowMap, sampleTypeShadows, txlSize);
        }

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
        color += IBL(surface, viewWS, specColor, diffuse, specular, integration);
    }

    // Reinhard operator. Supposedly preserves HDR values better
    color = color / (color + 1.f);
    color = pow(color, 1.f / 2.2f);
    return float4(color, 1.f);
}