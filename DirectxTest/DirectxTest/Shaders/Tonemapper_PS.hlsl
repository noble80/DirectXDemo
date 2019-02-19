#include "ACES.hlsl"

Texture2D sceneColor : register(t0);
Texture2D sceneDepth : register(t1);
SamplerState sampleTypeNearest : register(s3);

struct Quad
{
    float4 pos : SV_Position;
    float2 uv : TexCoord;
};

static const float exposure = 1.0f;

float4 main(Quad pIn) : SV_TARGET
{
    
    float3 color = sceneColor.Sample(sampleTypeNearest, pIn.uv).rgb;

    color = color / (color + 1.f);
    color = pow(color, 1.f / 2.2f);
    //float bw = (color.x + color.y + color.z) / 3.f;
    //color = lerp(color, bw, 1.0f);
    return float4(color, 1.f);
}