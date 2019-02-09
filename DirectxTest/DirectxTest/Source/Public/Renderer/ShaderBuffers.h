#pragma once
#include "stdafx.h"

#define POINTLIGHT_MAX 10
#define SPOTLIGHT_MAX 10


struct CSceneInfoBuffer
{
	DirectX::XMFLOAT4 time;
};

struct CTransformBuffer
{
	DirectX::XMMATRIX WorldViewProjection;
	DirectX::XMMATRIX Normal;
	DirectX::XMMATRIX World;
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

struct DirectionalShadowInfo
{
	DirectX::XMMATRIX viewProj;
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
	int pointLightCount;
	int spotLightCount;
	DirectX::XMFLOAT2 padding;
};

struct CLightInfoBuffer
{
	LightInfo lightInfo;
};