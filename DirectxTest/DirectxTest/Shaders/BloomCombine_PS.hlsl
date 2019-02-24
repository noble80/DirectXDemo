
Texture2D sceneColor : register(t0);
Texture2D sceneDepth : register(t1);
Texture2D sceneBloom : register(t2);
SamplerState samplerLinear : register(s2);

cbuffer BloomBuffer : register(b0)
{
    float threshold;
    float intensity;
    float2 padding;
};


struct Quad
{
    float4 pos : SV_Position;
    float2 uv : TexCoord;
};

float4 main(Quad pIn) : SV_TARGET
{
    float3 color = sceneColor.Sample(samplerLinear, pIn.uv).rgb;
    float3 bloom = sceneBloom.Sample(samplerLinear, pIn.uv).rgb * intensity;

    color += bloom;
    return float4(color, 1.f);
}