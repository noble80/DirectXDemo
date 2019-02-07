cbuffer ObjectBuffer : register(b0)
{
	matrix WorldViewProjection;
	matrix Normal;
	matrix World;
}

struct INPUT_VERTEX
{
	float3 Pos : POSITION;
};

struct OUTPUT_VERTEX
{
	float4 Pos : SV_POSITION;
};

OUTPUT_VERTEX main( INPUT_VERTEX vIn )
{
	OUTPUT_VERTEX output = (OUTPUT_VERTEX)0;
	const float4 Pos = float4(vIn.Pos, 1);
	output.Pos = mul(Pos, WorldViewProjection);
	return output;
}