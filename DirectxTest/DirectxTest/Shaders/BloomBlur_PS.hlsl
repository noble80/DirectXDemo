
Texture2D sceneColor : register(t0);
Texture2D sceneDepth : register(t1);
SamplerState samplerLinear : register(s2);

cbuffer BloomBuffer : register(b0)
{
    float2 resolution;
    int currMip;
    int horizontal;
    float threshold;
    float intensity;
    float2 padding;
};


struct Quad
{
    float4 pos : SV_Position;
    float2 uv : TexCoord;
};

static const float weight[5] = { 0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216 };

float4 main(Quad pIn) : SV_Target
{
    float3 color = sceneColor.Sample(samplerLinear, pIn.uv).xyz * weight[0];

    float2 txlSize = 1.0f / resolution;
    if (horizontal)
    {
        for (int i = 1; i < 5; i++)
        {
            color += sceneColor.Sample(samplerLinear, pIn.uv + float2(txlSize.x * i, 0.0f)).rgb * weight[i];
            color += sceneColor.Sample(samplerLinear, pIn.uv - float2(txlSize.x * i, 0.0f)).rgb * weight[i];
        }
    }
    else
    {
        for (int i = 1; i < 5; i++)
        {
            color += sceneColor.Sample(samplerLinear, pIn.uv + float2(0.f, txlSize.y * i)).rgb * weight[i];
            color += sceneColor.Sample(samplerLinear, pIn.uv - float2(0.f, txlSize.y * i)).rgb * weight[i];
        }
    }

    return float4(color, 1.0f);
}