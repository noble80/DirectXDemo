#define MAX_INSTANCES 500

cbuffer ObjectBuffer : register(b0)
{
    matrix ViewProjection;
    matrix _World[MAX_INSTANCES];
};

cbuffer SceneInfoBuffer : register(b5)
{
    float3 _EyePosition;
    float _Time;
};

struct InstancePos
{
    float4 position;
};

StructuredBuffer<InstancePos> positions : register(t0);

struct INPUT_VERTEX
{
    float3 Pos : POSITION;
    float2 Tex : TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
    uint instanceID : SV_InstanceID;
    //float instanceRand : INSTANCERAND;
};

struct OUTPUT_VERTEX
{
    float4 Pos : SV_POSITION;
    float3 PosWS : POSITION;
    float2 Tex : TEXCOORD0;
    float3 NormalWS : NORMAL;
    float3 TangentWS : TANGENT;
    float3 BinormalWS : BINORMAL;
    float linearDepth : DEPTH;
};


OUTPUT_VERTEX main(INPUT_VERTEX vIn)
{
    uint instanceID = vIn.instanceID;

    matrix World = _World[instanceID];
    World[3] = positions[instanceID].position;

    OUTPUT_VERTEX output = (OUTPUT_VERTEX) 0;
    const float4 Pos = float4(vIn.Pos, 1);
    output.PosWS = mul(Pos, World).xyz;
    output.Pos = mul(float4(output.PosWS, 1.0f), ViewProjection);

    output.Tex = vIn.Tex;

    output.TangentWS = mul(float4(vIn.Tangent, 0), World).xyz;
    output.TangentWS = normalize(output.TangentWS);
	
    output.BinormalWS = mul(float4(vIn.Binormal, 0), World).xyz;
    output.BinormalWS = normalize(output.BinormalWS);
	
    output.NormalWS = mul(float4(vIn.Normal, 0), World).xyz;
    output.NormalWS = normalize(output.NormalWS);

    output.linearDepth = output.Pos.w;

    return output;
}