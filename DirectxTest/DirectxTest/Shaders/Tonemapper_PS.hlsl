#include "ACES.hlsl"

Texture2D sceneColor : register(t0);
Texture2D sceneDepth : register(t1);
SamplerState sampleTypeNearest : register(s3);

struct Quad
{
    float4 pos : SV_Position;
    float2 uv : TexCoord;
};

cbuffer TonemapperBuffer : register(b0)
{
    float exposure;
    float BWStrength;
};

float4 main(Quad pIn) : SV_TARGET
{
    
    float3 color = sceneColor.Sample(sampleTypeNearest, pIn.uv).rgb;
    color *= exposure;
    color = color / (color + 1.f);
    color = pow(color, 1.f / 2.2f);
    float bw = dot(color.rgb, float3(0.2126f, 0.7152f, 0.0722f));
    color = lerp(color, bw, BWStrength);
    return float4(color, 1.f);
}