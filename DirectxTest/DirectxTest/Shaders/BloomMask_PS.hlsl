
Texture2D sceneColor : register(t0);
Texture2D sceneDepth : register(t1);
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
   
    float brightness = dot(color.rgb, float3(0.2126f, 0.7152f, 0.0722f));
    if (brightness > threshold)
        color = color.rgb;
    else
        color = 0.f;

    return float4(color, 1.0f);
}