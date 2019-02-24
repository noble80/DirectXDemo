#pragma once

#include "Common.hlsl"


cbuffer ObjectBuffer : register(b0)
{
    matrix ViewProjection;
    matrix Normal;
    matrix World;
    matrix WorldView;
    matrix Projection;
};

cbuffer SceneInfoBuffer : register(b5)
{
    float3 _EyePosition;
    float _Time;
};