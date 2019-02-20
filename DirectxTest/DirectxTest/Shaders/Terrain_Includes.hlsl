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


cbuffer CTerrainBuffer : register(b7)
{
    float _TessellationMinDistance;
    float _TessellationMaxDistance;

    
	// In 2^X increments
    float _TessellationMinFactor;
    float _TessellationMaxFactor;
    
    float2 _TextureTiling;
    float _TxlSizeU;
    float _TxlSizeV;
    float _PatchSpacing;
};

Texture2D HeightMap : register(t7);

