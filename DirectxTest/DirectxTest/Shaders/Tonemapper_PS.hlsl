
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
    float time;
    int warp;
};

float4 main(Quad pIn) : SV_TARGET
{
    float3 color;
    if (warp)
    {
        float2 uv = pIn.uv - 0.5f;
        float x = radians(uv * 180.f) * 5.f;
        float2 offset = float2(1.f, 1.f + sin(x + time*0.2f) * 0.2f);
        color = sceneColor.Sample(sampleTypeNearest, uv * offset + 0.5f).rgb;
    }
    else
    {
        color = sceneColor.Sample(sampleTypeNearest, pIn.uv).rgb;
    }
    color *= exposure;
    color = color / (color + 1.f);
    color = pow(color, 1.f / 2.2f);
    float bw = dot(color.rgb, float3(0.2126f, 0.7152f, 0.0722f));
    color = lerp(color, bw, BWStrength);

   

    return float4(color, 1.f);
}