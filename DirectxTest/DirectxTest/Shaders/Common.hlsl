#pragma once

#define POINTLIGHT_MAX 10
#define SPOTLIGHT_MAX 10

struct PointLight
{
	float3 position;
	float radius;
	float3 color;
	float padding;
};

struct DirectionalLight
{
	float3 direction;
    float padding1;
	float3 color;
	float padding2;
};

struct DirectionalShadowInfo
{
	matrix viewProj;
	float bias;
	float normalOffset;
	float resolution;
	float padding;
};

struct SpotLight
{
	float3 position;
	float radius;
	float3 color;
	float innerCone;
	float3 direction;
	float outerCone;
};

struct LightInfo
{
	PointLight pointLights[POINTLIGHT_MAX];
	SpotLight spotLights[SPOTLIGHT_MAX];
	DirectionalLight directionalLight;
    DirectionalShadowInfo directionalShadowInfo;
	int pointLightCount;
	int spotLightCount;
	float2 padding;
};

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

float4 bdrf(float3 normal, float3 normalWS, float3 positionWS, float3 textureColor, float ambient, LightInfo info, Texture2D directionalShadowMap, SamplerComparisonState shadowSampler)
{
    float3 output = float3(0.f, 0.f, 0.f);

    for (int i = 0; i < info.pointLightCount; i++)
    {
        float radius = info.pointLights[i].radius;
		float3 lightPos = info.pointLights[i].position;
        float3 dir = normalize(lightPos - positionWS);
        float dist = length(lightPos - positionWS);
        float lightRatio = saturate(dot(normal, dir));
        float attenuation = saturate(1.0f - dist * dist / (radius * radius));
        attenuation *= attenuation;
        output += info.pointLights[i].color * lightRatio * attenuation;
    }

    for (i = 0; i < info.spotLightCount; i++)
    {
        float radius = info.spotLights[i].radius;
        float outerCone = info.spotLights[i].outerCone;
        float innerCone = info.spotLights[i].innerCone;
        float3 lightPos = info.spotLights[i].position;
        float3 dir = normalize(lightPos - positionWS);
        float dist = length(lightPos - positionWS);

        float lightRatio = saturate(dot(normal, dir));
        float pAttenuation = saturate(1.0f - dist * dist / (radius * radius));
        pAttenuation *= pAttenuation;

        float sAttenuation = saturate((dot(dir, -info.spotLights[i].direction) - outerCone) / (innerCone - outerCone));
        sAttenuation *= sAttenuation;

        output += info.spotLights[i].color * lightRatio * sAttenuation * pAttenuation;
    }

	{
        float shadowRatio = 1.0f;
        float txlSize = 1.0f / (info.directionalShadowInfo.resolution);

        float lightRatio = saturate(dot(normal, -info.directionalLight.direction));
        float cosAngle = saturate(1.f - dot(normalWS, -info.directionalLight.direction));
        float3 normalOffset = (info.directionalShadowInfo.normalOffset * cosAngle) * normalWS;

        float4 lightSpacePos = mul(float4(positionWS + normalOffset, 1), info.directionalShadowInfo.viewProj);
        float3 lspProj = lightSpacePos.xyz / lightSpacePos.w;
		// Screencoords to NDC
        float2 shadowCoords;
        shadowCoords.x = lspProj.x / 2.0f + 0.5f;
        shadowCoords.y = -lspProj.y / 2.0f + 0.5f;
		//Check if within shadowmap bounds
        if ((saturate(shadowCoords.x) == shadowCoords.x) && (saturate(shadowCoords.y) == shadowCoords.y))
        {
			//Get depth w/ bias
            float z = lspProj.z - info.directionalShadowInfo.bias;

            shadowRatio = PCFBlur(shadowCoords, 2, z, directionalShadowMap, shadowSampler, txlSize);
        }

        output += info.directionalLight.color * lightRatio * shadowRatio;
    }

    output = output * textureColor + textureColor * ambient;

    return float4(output, 1.f);
}