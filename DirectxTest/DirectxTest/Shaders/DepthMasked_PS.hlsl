Texture2D diffuseMap : register(t0);
SamplerState linearWrap : register(s0);

struct INPUT_PIXEL
{
    float4 Pos : SV_POSITION;
    float3 PosWS : POSITION;
    float2 Tex : TEXCOORD0;
    float3 NormalWS : NORMAL;
    float3 TangentWS : TANGENT;
    float3 BinormalWS : BINORMAL;
    float linearDepth : DEPTH;
};

float4 main(INPUT_PIXEL pIn) : SV_TARGET
{
    float a = diffuseMap.Sample(linearWrap, pIn.Tex).a;
    clip(a < 0.1f ? -1 : 1);
    return 1.f;
}