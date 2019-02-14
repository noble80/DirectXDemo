
struct Quad
{
    float4 pos : SV_Position;
    float2 uv : TexCoord;
};

Quad main(uint index : SV_VERTEXID)
{
    Quad output;
	//Trick to not vertices for fullscreen quad
    output.uv = float2(index & 1, index >> 1);
    output.pos = float4((output.uv.x - 0.5f) * 2, -(output.uv.y - 0.5f) * 2, 0.f, 1.f);
    return output;
}

