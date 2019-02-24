
Texture2D sceneColor : register(t0);
Texture2D sceneDepth : register(t1);
Texture2D sceneBlur : register(t2);
SamplerState samplerLinear : register(s2);

cbuffer DOFBuffer : register(b0)
{
    float _nearZ;
    float _farZ;
    float _DOFStart;
    float _DOFFalloff;
};


struct Quad
{
    float4 pos : SV_Position;
    float2 uv : TexCoord;
};

float4 main(Quad pIn) : SV_TARGET
{
    float3 color = sceneColor.Sample(samplerLinear, pIn.uv).rgb;
    float3 blur = sceneBlur.Sample(samplerLinear, pIn.uv).rgb;
    float z;
    z = _nearZ * _farZ / (_farZ - sceneDepth.Sample(samplerLinear, pIn.uv).r * (_farZ - _nearZ));
    z = clamp((z - _DOFStart) / _DOFFalloff, 0.f, 1.0f);

    color = lerp(color, blur, z);
    return float4(color, 1.f);
}