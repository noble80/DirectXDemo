TextureCube skyCubemap : register(t0);
SamplerState samplerSky : register(s2);

struct INPUT_PIXEL
{
    float4 Pos : SV_POSITION;
    float3 PosLS : POSITION;
};

float4 main(INPUT_PIXEL pIn) : SV_TARGET
{
    float3 color = skyCubemap.Sample(samplerSky, pIn.PosLS).xyz;
    //color = pow(color, 1.f / 2.2f);

    return float4(color, 1.f);

}