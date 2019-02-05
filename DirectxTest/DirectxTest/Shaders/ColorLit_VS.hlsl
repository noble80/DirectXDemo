cbuffer ObjectBuffer : register(b0)
{
	matrix WorldViewProjection;
	matrix NormalView;
	matrix World;
}

struct INPUT_VERTEX
{
	float3 Pos : POSITION;
	float3 Normal : NORMAL;
};

struct OUTPUT_VERTEX
{
	float4 Pos : SV_POSITION;
	float3 PosWS : POSITION;
	float3 NormalVS : NORMAL0;
	float3 NormalWS : NORMAL1;
};


OUTPUT_VERTEX main( INPUT_VERTEX vIn )
{
	float normal, tangent, binormal;

	OUTPUT_VERTEX output = (OUTPUT_VERTEX)0;
	const float4 Pos = float4(vIn.Pos, 1);

	output.NormalVS = mul(float4(vIn.Normal, 0), NormalView).xyz;
	output.NormalVS = normalize(output.NormalVS);

	output.Pos = mul(Pos, WorldViewProjection);
	output.PosWS = mul(Pos, World).xyz;
	output.NormalWS = mul(float4(vIn.Normal, 0), World).xyz;
	output.NormalWS = normalize(output.NormalWS);


	return output;
}