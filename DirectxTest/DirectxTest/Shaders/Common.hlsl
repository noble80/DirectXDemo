#pragma once

#define POINTLIGHT_MAX 10
#define SPOTLIGHT_MAX 10

static const float PI = 3.14159265f;



struct PointLight
{
    float3 position;
    float radius;
    float3 color;
    float padding;
};

struct DirectionalLight
{
    float3 direction;
    float padding1;
    float3 color;
    float padding2;
};

struct DirectionalShadowInfo
{
    matrix viewProj;
    float bias;
    float normalOffset;
    float resolution;
    float padding;
};

struct SpotLight
{
    float3 position;
    float radius;
    float3 color;
    float innerCone;
    float3 direction;
    float outerCone;
};

struct LightInfo
{
    PointLight pointLights[POINTLIGHT_MAX];
    SpotLight spotLights[SPOTLIGHT_MAX];
    DirectionalLight directionalLight;
    DirectionalShadowInfo directionalShadowInfo;
    float3 ambientColor;
    int pointLightCount;
    int spotLightCount;
    float3 padding;
};

float remap(float value, float low1, float high1, float low2, float high2)
{
    return low2 + (value - low1) * (high2 - low2) / (high1 - low1);
}

float samplePCF(float2 uv, float x, float y, float z0, Texture2D shadowMap, SamplerComparisonState samp, float txlSize)
{
    float2 coords = uv + float2(x, y) * txlSize;
    float z = z0;

    return shadowMap.SampleCmpLevelZero(samp, coords, z);
}

float PCFBlur(float2 uv, int samples, float z0, Texture2D shadowMap, SamplerComparisonState samp, float txlSize)
{
    float output = 0.f;
    for (int u = -samples; u <= samples; u++)
    {
        for (int v = -samples; v <= samples; v++)
        {
            output += samplePCF(uv, u, v, z0, shadowMap, samp, txlSize);
        }
    }

    output /= (samples * 2.f + 1.f) * (samples * 2.f + 1.f);

    return output;
}

// Check for texture flags
inline int HasDiffuseTexture(int flags)
{
    return ((flags & 1) > 0 ? 1 : 0);
}
inline int HasSpecularTexture(int flags)
{
    return ((flags & 2) > 0 ? 1 : 0);
}
inline int HasMetallicTexture(int flags)
{
    return ((flags & 2) > 0 ? 1 : 0);
}
inline int HasGlossinessTexture(int flags)
{
    return ((flags & 4) > 0 ? 1 : 0);
}
inline int HasRoughnessTexture(int flags)
{
    return ((flags & 4) > 0 ? 1 : 0);
}
inline int HasNormalTexture(int flags)
{
    return ((flags & 8) > 0 ? 1 : 0);
}
inline int HasAOTexture(int flags)
{
    return ((flags & 16) > 0 ? 1 : 0);
}
inline int HasEmissiveMask(int flags)
{
    return ((flags & 32) > 0 ? 1 : 0);
}

inline int HasReflections(int flags)
{
    return ((flags & 64) > 0 ? 1 : 0);
}




