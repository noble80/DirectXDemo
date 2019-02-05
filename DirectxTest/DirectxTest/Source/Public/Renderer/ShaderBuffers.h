#pragma once
#include "stdafx.h"

struct CMatricesBuffer
{
	DirectX::XMMATRIX WorldViewProjection;
	DirectX::XMMATRIX NormalView;
	DirectX::XMMATRIX World;
};

struct CLightBuffer
{
	DirectX::XMFLOAT4 lightColor;
	DirectX::XMFLOAT4 lightDirAndIntensity;
	DirectX::XMFLOAT4 BiasOffsetRes;
	DirectX::XMMATRIX Shadow;
};