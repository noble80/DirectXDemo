#pragma once
#include "GraphicsStructures.h"

struct Texture2D : Resource
{
	ID3D11Resource* texture;
	ID3D11ShaderResourceView* resourceView;

	virtual ~Texture2D() override;

	virtual void Release() override;
};