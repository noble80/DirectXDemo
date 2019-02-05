#pragma once
#include "GraphicsStructures.h"

struct Texture2D : Resource
{
	ID3D11Resource* d3dresource;
	ID3D11ShaderResourceView* shaderResourceView;

	virtual ~Texture2D() override;

	virtual void Release() override;
};