#include "PSIncludes.hlsl"

struct SurfacePBR
{
    float3 diffuseColor;
    float metallic;
    float3 emissiveColor;
    float roughness;
    float3 normal;
    float ambient;
};

// using Schlick's approximation for calculating fresnel. K is a constant for non metallics
float3 FresnelSchlick(float3 H, float3 V, float3 specColor)
{
    float cosAngle = max(dot(H, V), 0.f);

    return specColor + (1.f - specColor) * pow(1.f - cosAngle, 5.f);
}

// Variation using only roughness for IBL calculations
float3 FresnelSchlickEnvironment(float3 N, float3 V, float3 specColor, float roughness)
{
    float cosAngle = max(dot(N, V), 0.f);

    return specColor + (max(1.f - roughness, specColor) - specColor) * pow(1.f - cosAngle, 5.f);
}

//Specular based on Cook-Torrance model
float NormalDistribution(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.f);
    float NdotH2 = NdotH * NdotH;
	
    float d = (NdotH2 * (a2 - 1.f) + 1.f);
    d = PI * d * d;
	
    return a2 / d;
}


float GeometryTermSchlick(float NdotV, float roughness)
{
    float r = (roughness + 1.f);
    float k = (r * r) / 8.f;
    
    float num = NdotV;
    float den = NdotV * (1.f - k) + k;
	
    return num / den;
}
float GeometryTermSmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.f);
    float NdotL = max(dot(N, L), 0.f);
    float term1 = GeometryTermSchlick(NdotL, roughness);
    float term2 = GeometryTermSchlick(NdotV, roughness);
	
    return term1 * term2;
}

float3 IBL(SurfacePBR surface, float3 viewWS, float3 specColor, float3 IBLDiffuse, float3 IBLSpecular, float2 IBLIntegration)
{
    float3 N = surface.normal;
    float3 V = -viewWS;

    float3 F = FresnelSchlickEnvironment(N, V, specColor, surface.roughness);
    float3 kD = 1.f - F;
    kD *= 1.f - surface.metallic;
  
    float3 diffuse = surface.diffuseColor * IBLDiffuse;
    float3 specular = IBLSpecular * (F * IBLIntegration.x + IBLIntegration.y);
  
    return (kD * diffuse + specular) * surface.ambient;
}

float3 PBR(SurfacePBR surface, float3 lightDir, float3 viewWS, float3 specColor)
{
    float3 N = surface.normal;
    float3 L = lightDir;
    float3 V = -viewWS;
    float3 H = normalize(L + V);

    // Cook-Torrance FGD/4nlnv
    float D = NormalDistribution(N, H, surface.roughness);
    float3 F = FresnelSchlick(H, V, specColor);
    float G = GeometryTermSmith(N, V, L, surface.roughness);
        
    float3 kD = 1.f - F;
    kD *= 1.f - surface.metallic;
        
    float3 num = D * F * G;
    float den = 4.f * max(dot(N, V), 0.f) * max(dot(N, L), 0.f);
    // limit denominator to 0.001 to prevent divisions by 0
    float3 specular = num / max(den, 0.001f);
            
    float NdotL = max(dot(N, L), 0.f);

    return NdotL * (kD * surface.diffuseColor / PI + specular);
}
