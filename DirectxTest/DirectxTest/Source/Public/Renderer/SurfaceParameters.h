#pragma once
#include "stdafx.h"

#define SURFACE_FLAG_HAS_DIFFUSE_MAP 0b1
#define SURFACE_FLAG_HAS_SPECULAR_MAP 0b10
#define SURFACE_FLAG_HAS_METALLIC_MAP 0b10
#define SURFACE_FLAG_HAS_GLOSSINESS_MAP 0b100
#define SURFACE_FLAG_HAS_ROUGHNESS_MAP 0b100
#define SURFACE_FLAG_HAS_NORMAL_MAP 0b1000
#define SURFACE_FLAG_HAS_AO_MAP 0b10000
#define SURFACE_FLAG_HAS_EMISSIVE_MASK 0b100000
#define SURFACE_FLAG_HAS_REFLECTIONS 0b1000000
#define SURFACE_FLAG_IS_MASKED 0b10000000
#define SURFACE_FLAG_IS_TRANSLUSCENT 0b100000000

#define SURFACE_FLAG_HAS_DETAILS_MAP (SURFACE_FLAG_HAS_SPECULAR_MAP | SURFACE_FLAG_HAS_GLOSSINESS_MAP | SURFACE_FLAG_HAS_AO_MAP)

struct SurfaceProperties
{
	DirectX::XMFLOAT3 diffuseColor;
	union
	{
		float specularIntensity;
		float metallic;
	};
	DirectX::XMFLOAT3 emissiveColor;
	union
	{
		float glossiness;
		float roughness;
	};
	float normalIntensity;
	float ambientIntensity;
	int textureFlags;
	float padding;

	SurfaceProperties()
	{		
		specularIntensity = 0.1f;
		glossiness = 0.5f;
		normalIntensity = 1.5f;
		ambientIntensity = 1.0f;
		diffuseColor = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
		emissiveColor = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		textureFlags = 0;
	}
};