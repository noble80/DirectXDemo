#pragma once
#include "stdafx.h"

#include "SurfaceParameters.h"

#define POINTLIGHT_MAX 10
#define SPOTLIGHT_MAX 10
#define CASCADES_MAX 5

struct CSceneInfoBuffer
{
	DirectX::XMFLOAT3 eyePosition;
	float time;
};

struct CTransformBuffer
{
	DirectX::XMMATRIX ViewProjection;
	DirectX::XMMATRIX Normal;
	DirectX::XMMATRIX World;
	DirectX::XMMATRIX WorldView;
	DirectX::XMMATRIX Projection;
};

struct PointLight
{
	DirectX::XMFLOAT3 position;
	float radius;
	DirectX::XMFLOAT3 color;
	float padding;
};

struct DirectionalLight
{
	DirectX::XMFLOAT3 direction;
	float padding1;
	DirectX::XMFLOAT3 color;
	float padding2;
};

struct Cascade
{
	DirectX::XMMATRIX lightSpace;
	float cascadeSplit;
	DirectX::XMFLOAT3 padding;
};

struct DirectionalShadowInfo
{
	Cascade cascades[CASCADES_MAX];
	float bias;
	float normalOffset;
	float resolution;
	float padding;
};


struct SpotLight
{
	DirectX::XMFLOAT3 position;
	float radius;
	DirectX::XMFLOAT3 color;
	float innerCone;
	DirectX::XMFLOAT3 direction;
	float outerCone;
};

struct LightInfo
{
	PointLight pointLights[POINTLIGHT_MAX];
	SpotLight spotLights[SPOTLIGHT_MAX];
	DirectionalLight directionalLight;
	DirectionalShadowInfo directionalShadowInfo;
	DirectX::XMFLOAT3 ambientColor;
	int pointLightCount;
	int spotLightCount;
	DirectX::XMFLOAT3 padding;
};

struct CLightInfoBuffer
{
	LightInfo lightInfo;
};