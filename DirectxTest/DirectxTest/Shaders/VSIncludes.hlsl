#pragma once

#include "Common.hlsl"


cbuffer ObjectBuffer : register(b0)
{
    matrix WorldViewProjection;
    matrix Normal;
    matrix World;
};

cbuffer SceneInfoBuffer : register(b5)
{
    float3 _EyePosition;
    float _Time;
};