#pragma once


float samplePCF(float2 uv, float x, float y, float z0, Texture2D shadowMap, SamplerComparisonState samp,  float txlSize)
{
	float2 coords = uv + float2(x, y) * txlSize;
	float z = z0;

	return shadowMap.SampleCmpLevelZero(samp, coords, z);
}

float PCFBlur(float2 uv, int samples, float z0, Texture2D shadowMap, SamplerComparisonState samp, float txlSize)
{
	float output = 0.f;
	for(int u = -samples; u <= samples; u++)
	{
		for(int v = -samples; v <= samples; v++)
		{
			output += samplePCF(uv, u, v, z0, shadowMap, samp, txlSize);
		}
	}

	output /= (samples*2.f + 1.f)*(samples * 2.f + 1.f);

	return output;
}