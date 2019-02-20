#include "Constants.hlsl"

#define POINTLIGHT_MAX 10
#define SPOTLIGHT_MAX 10
#define CASCADES_MAX 5

SamplerState sampleTypeWrap : register(s0);
SamplerComparisonState sampleTypeShadows : register(s1);
SamplerState sampleTypeClamp : register(s2);
SamplerState sampleTypeNearest : register(s3);

Texture2DArray cascadeShadowmap : register(t6);


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

struct Cascade
{
    matrix lightSpace;
    float cascadeSplit;
    float3 padding;
};

struct DirectionalShadowInfo
{
    Cascade cascades[CASCADES_MAX];
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

float samplePCF(float2 uv, int currCascade, float2 offset, float z0, float txlSize)
{
    float2 coords = uv + offset * txlSize;
    return cascadeShadowmap.SampleCmpLevelZero(sampleTypeShadows, float3(coords, currCascade), z0);
}

float2 randomFloat(float4 seed)
{
    float d = dot(seed, float4(12.9898, 78.233, 45.164, 94.673));
    return frac(sin(d) * 43758.5453);
}

float PCFBlur(float2 uv, int currCascade, int samples, float z0, float txlSize, float3 seed)
{
    float output = 0.f;
    for (int i = 0; i < samples; ++i)
    {
        float rand = randomFloat(float4(seed, 0.f)) ;
        float2 jitterFactor = rand*2.f - 1.f;
        float2 offset = PoissonSamples[i] * 10.f / (currCascade + 1) + jitterFactor * 1.f / (currCascade + 1);
        output += samplePCF(uv, currCascade, offset, z0, txlSize);
    }
    output /= samples;
    //output = samplePCF(uv, 0, z0, shadowMap, txlSize);
    //output = currCascade / CASCADES_MAX;

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

inline int IsMasked(int flags)
{
    return ((flags & 128) > 0 ? 1 : 0);
}

inline int IsTransluscent(int flags)
{
    return ((flags & 256) > 0 ? 1 : 0);
}




