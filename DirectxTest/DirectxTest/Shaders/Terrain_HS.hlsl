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

float GetPostProjectionSphereExtent(float3 Origin, float Diameter)
{
    float4 ClipPos = mul(float4(Origin, 1.0), ViewProjection);
    return abs((Diameter * _TesselationProj) / ClipPos.w);
}

float CalculateTessellationFactor(float3 Control0, float3 Control1)
{
    float e0 = distance(Control0, Control1);
    float3 m0 = (Control0 + Control1) / 2;
    return max(1, _TesselationFactor * GetPostProjectionSphereExtent(m0, e0));
}

// Patch Constant Function
HullConstantDataOut CalcHSPatchConstants(
	InputPatch<VertexOut, NUM_CONTROL_POINTS> ip,
	uint PatchID : SV_PrimitiveID)
{
    HullConstantDataOut hCDOut;
        
    hCDOut.EdgeTessFactor[0] = CalculateTessellationFactor(ip[0].PosL, ip[3].PosL);
    hCDOut.EdgeTessFactor[1] = CalculateTessellationFactor(ip[0].PosL, ip[1].PosL);
    hCDOut.EdgeTessFactor[2] = CalculateTessellationFactor(ip[1].PosL, ip[2].PosL);
    hCDOut.EdgeTessFactor[3] = CalculateTessellationFactor(ip[3].PosL, ip[2].PosL);
        
    hCDOut.InsideTessFactor[0] = 0.25 * (hCDOut.EdgeTessFactor[0] + hCDOut.EdgeTessFactor[1] + hCDOut.EdgeTessFactor[2] + hCDOut.EdgeTessFactor[3]);
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
