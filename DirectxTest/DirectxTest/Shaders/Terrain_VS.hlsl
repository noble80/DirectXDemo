#include "Terrain_Includes.hlsl"

struct VertexIn
{
    float3 PosL : POSITION;
    float2 Tex : TEXCOORD0;
};

struct VertexOut
{
    float3 PosL : POSITION;
    float2 Tex : TEXCOORD0;
};

VertexOut main(VertexIn vIn)
{
    VertexOut vOut;
    
    vOut.PosL = vIn.PosL;
    vOut.PosL.y = (HeightMap.SampleLevel(sampleTypeClamp, vIn.Tex, 0).r - 0.5f) * 2625.f;
    vOut.Tex = vIn.Tex;
    
    return vOut;
}