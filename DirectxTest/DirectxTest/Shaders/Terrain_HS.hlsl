#include "Terrain_Includes.hlsl"
// Input control point
struct VertexOut
{
    float3 PosL : POSITION;
    float2 Tex : TEXCOORD0;
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

float GetEdgeTessFactor(float3 p)
{
    float dist = distance(p, _EyePosition);    
    float ratio = saturate((dist - _TessellationMinDistance) / (_TessellationMaxDistance - _TessellationMinDistance));    
    return pow(2, (lerp(_TessellationMaxFactor, _TessellationMinFactor, ratio)));
}

// Patch Constant Function
HullConstantDataOut CalcHSPatchConstants(
	InputPatch<VertexOut, NUM_CONTROL_POINTS> ip,
	uint PatchID : SV_PrimitiveID)
{
    HullConstantDataOut hCDOut;

	// calculate tesselation factor based on patch edges
    float3 e0 = 0.5f * (ip[0].PosL + ip[3].PosL);
    float3 e1 = 0.5f * (ip[0].PosL + ip[1].PosL);
    float3 e2 = 0.5f * (ip[1].PosL + ip[2].PosL);
    float3 e3 = 0.5f * (ip[3].PosL + ip[2].PosL);
    float3 c = 0.25f * (ip[0].PosL + ip[1].PosL + ip[3].PosL + ip[2].PosL);
        
    hCDOut.EdgeTessFactor[0] = GetEdgeTessFactor(e0);
    hCDOut.EdgeTessFactor[1] = GetEdgeTessFactor(e1);
    hCDOut.EdgeTessFactor[2] = GetEdgeTessFactor(e2);
    hCDOut.EdgeTessFactor[3] = GetEdgeTessFactor(e3);
        
    hCDOut.InsideTessFactor[0] = GetEdgeTessFactor(c);
    hCDOut.InsideTessFactor[1] = hCDOut.InsideTessFactor[0];

    return hCDOut;
}

[domain("quad")]
[partitioning("fractional_even")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("CalcHSPatchConstants")]
[maxtessfactor(64.0f)]
HullOut main(
	InputPatch<VertexOut, NUM_CONTROL_POINTS> ip,
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID)
{
    HullOut hOut;
	// Insert code to compute Output here
    hOut.PosL = ip[i].PosL;
    hOut.Tex = ip[i].Tex;
    return hOut;
}
