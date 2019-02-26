#define MAX_INSTANCES 500

struct InstancePos
{
    float4 position;
};

cbuffer ObjectBuffer : register(b0)
{
    matrix ViewProjection;
    matrix _World[MAX_INSTANCES];
};

SamplerState sampleTypeClamp : register(s2);
Texture2D HeightMap : register(t7);
RWStructuredBuffer<InstancePos> BufferOut : register(u0);

[numthreads(500, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float4 _pos = _World[DTid.x][3];
    float2 dimensions = 8000;
    float height = 2625.f;
    float2 pos = float2(_pos[0], _pos[2]);
    
    float2 uv = pos / (dimensions / 2) *0.5f + 0.5f;
    uv.y = 1.f - uv.y;

    float y = (HeightMap.SampleLevel(sampleTypeClamp, uv, 0).r - 0.5f) * height;
    
    BufferOut[DTid.x].position = _pos;
    BufferOut[DTid.x].position[1] = y;

}