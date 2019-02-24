
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
    float3 _fogColor;
    float _fogAlpha;
    float _nearZ;
    float _farZ;
    float _fogStart;
    float _fogFallOff;
};

float4 main(Quad pIn) : SV_TARGET
{
    float z;
    float3 color = sceneColor.Sample(sampleTypeNearest, pIn.uv).rgb;
    z = _nearZ * _farZ / (_farZ - sceneDepth.Sample(sampleTypeNearest, pIn.uv).r * (_farZ - _nearZ));
    z = clamp((z - _fogStart) / _fogFallOff, 0.f, _fogAlpha);

    color = lerp(color, _fogColor, z);

    return float4(color, 1.f);
}