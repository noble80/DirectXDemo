

// QUAD bilinear interpolation
float2 Bilerp(float2 v[4], float2 uv)
{
    // bilerp the float2 values
    float2 side1 = lerp(v[0], v[1], uv.x);
    float2 side2 = lerp(v[3], v[2], uv.x);
    float2 result = lerp(side1, side2, uv.y);
	
    return result;
}

float3 Bilerp(float3 v[4], float2 uv)
{
    // bilerp the float3 values
    float3 side1 = lerp(v[0], v[1], uv.x);
    float3 side2 = lerp(v[3], v[2], uv.x);
    float3 result = lerp(side1, side2, uv.y);
	
    return result;
}

float4 Bilerp(float4 v[4], float2 uv)
{
    // bilerp the float4 values
    float4 side1 = lerp(v[0], v[1], uv.x);
    float4 side2 = lerp(v[3], v[2], uv.x);
    float4 result = lerp(side1, side2, uv.y);
	
    return result;
}

float2 BarycentricInterpolate(float2 v0, float2 v1, float2 v2, float3 barycentric)
{
    return barycentric.z * v0 + barycentric.x * v1 + barycentric.y * v2;
}

float3 BarycentricInterpolate(float3 v0, float3 v1, float3 v2, float3 barycentric)
{
    return barycentric.z * v0 + barycentric.x * v1 + barycentric.y * v2;
}

float4 BarycentricInterpolate(float4 v0, float4 v1, float4 v2, float3 barycentric)
{
    return barycentric.z * v0 + barycentric.x * v1 + barycentric.y * v2;
}