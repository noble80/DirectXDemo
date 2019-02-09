#include "PSIncludes.hlsl"

struct INPUT_PIXEL
{
	float4 Pos : SV_POSITION;
};

float4 main(INPUT_PIXEL pIn) : SV_TARGET
{
    return float4(1.f, 0.f, 0.f, 1.f);
}