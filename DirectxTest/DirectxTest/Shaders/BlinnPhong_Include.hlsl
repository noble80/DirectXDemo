#include "PSIncludes.hlsl"

struct SurfaceBlinnPhong
{
    float3 diffuseColor;
    float specularIntensity;
    float3 emissiveColor;
    float glossiness;
    float3 normal;
    float ambient;
};

float3 BlinnPhong(SurfaceBlinnPhong surface, float3 lightDir, float3 viewWS)
{

    float specularPower = pow(2, surface.glossiness * 10.f);
    float specularIntensity = surface.specularIntensity;
    float3 normal = surface.normal;
    float LDotN = max(0.0f, dot(lightDir, normal));
    float3 diffuse = surface.diffuseColor * LDotN;
    float3 H = normalize(lightDir - viewWS);
    float specular = specularIntensity * pow(saturate(dot(H, normal)), specularPower);

    float specularNormalizationFactor = (specularPower + 8) / (8 * PI);

    return (diffuse / PI + specular * specularNormalizationFactor) * LDotN;
}
