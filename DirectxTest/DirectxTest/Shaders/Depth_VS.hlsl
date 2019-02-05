cbuffer ConstantBuffer : register(b0)
{
	matrix WorldViewProjection;
}

struct INPUT_VERTEX
{
	float3 position : POSITION;
};

struct VS_OUTPUT
{
	float4 pos    : SV_POSITION;
};

VS_OUTPUT main( INPUT_VERTEX vIn )
{
	VS_OUTPUT output;
	output.pos = float4(vIn.position, 1);
	output.pos = mul(output.pos , WorldViewProjection);

	return output;
}