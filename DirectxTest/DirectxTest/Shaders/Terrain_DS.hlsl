#include "Terrain_Includes.hlsl"
#include "Interpolation.hlsl"

struct DomainOutput
{
	float4 Pos  : SV_POSITION;
	// TODO: change/add other stuff
    float3 PosWS : POSITION;
    float2 Tex : TEXCOORD0;
    float2 Tex2 : TEXCOORD1;
    float linearDepth : DEPTH;
};

// Output control point
struct HullOut
{
    float3 PosL : POSITION;
    float2 Tex : TEXCOORD0;

};

// Output patch constant data.
struct HullConstantDataOut
{
    float EdgeTessFactor[4] : SV_TessFactor; // e.g. would be [4] for a quad domain
    float InsideTessFactor[2] : SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
	// TODO: change/add other stuff
};

#define NUM_CONTROL_POINTS 4

[domain("quad")]
DomainOutput main(
	HullConstantDataOut input,
	float2 domain : SV_DomainLocation,
	const OutputPatch<HullOut, NUM_CONTROL_POINTS> patch)
{
    DomainOutput dOut;

    float3 pws[4] = { patch[0].PosL, patch[1].PosL, patch[2].PosL, patch[3].PosL };
    float2 texs[4] = { patch[0].Tex, patch[1].Tex, patch[2].Tex, patch[3].Tex };

    dOut.Tex = Bilerp(texs, domain);
    dOut.Tex2 = dOut.Tex * _TextureTiling;

    float3 pos = Bilerp(pws, domain);
    pos.y = (HeightMap.SampleLevel(sampleTypeClamp, dOut.Tex, 0).r - 0.5f) * 2625.f;

    dOut.PosWS = mul(float4(pos, 1.0f), World);
    dOut.Pos = mul(float4(dOut.PosWS, 1.0f), ViewProjection);
    dOut.linearDepth = dOut.Pos.w;

    return dOut;
}
