#pragma once
#include "Common.hlsl"

cbuffer SceneInfoBuffer : register(b5)
{
	float3 _EyePosition;
	float _Time;
};

cbuffer LightInfoBuffer : register(b6)
{
	LightInfo lightInfo;
};


cbuffer CSurfaceProperties : register(b4)
{
    float3 _diffuseColor;
    float _specularIntensity;
    float3 _emissiveColor;
    float _glossinessRoughness;
    float _normalIntensity;
    float _ambientIntensity;
    int _textureFlags;
    float padding;
};