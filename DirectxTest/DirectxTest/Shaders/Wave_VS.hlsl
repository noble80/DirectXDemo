cbuffer ObjectBuffer : register(b0)
{
	matrix WorldViewProjection;
	matrix Normal;
	matrix World;
}

cbuffer SceneInfoBuffer : register(b5)
{
    float4 time;
};

struct INPUT_VERTEX
{
	float3 Pos : POSITION;
	float3 Normal : NORMAL;
    float2 Tex : TEXCOORD0;
};

struct OUTPUT_VERTEX
{
	float4 Pos : SV_POSITION;
	float3 PosWS : POSITION;
	float3 NormalWS : NORMAL;
    float2 Tex : TEXCOORD0;
};


OUTPUT_VERTEX main( INPUT_VERTEX vIn )
{
	OUTPUT_VERTEX output = (OUTPUT_VERTEX)0;
	float4 Pos = float4(vIn.Pos, 1);
    float alpha = (sin(time.x) + 1) / 2.f;
    alpha *= (sin(Pos.y * 10.f) + 1) / 2.f;
    alpha *= (sin(Pos.z * 10.f) + 1) / 2.f;
    alpha *= (sin(Pos.x * 10.f) + 1) / 2.f;

    float waveDis = lerp(-0.5f, 0.5f, alpha);
    Pos += float4(waveDis*vIn.Normal, 0.f);
	output.PosWS = mul(Pos, World).xyz;

    output.Tex = vIn.Tex;

    output.NormalWS = mul(float4(vIn.Normal, 0), Normal).xyz;
	output.NormalWS = normalize(output.NormalWS);

	output.Pos = mul(Pos, WorldViewProjection);

	return output;
}