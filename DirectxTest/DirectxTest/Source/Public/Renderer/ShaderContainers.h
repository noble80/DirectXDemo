#pragma once

#include "GraphicsStructures.h"

struct VertexShader : public Resource
{
	VertexShader();
	virtual ~VertexShader() override;
	virtual void Release() override;

	ID3D11VertexShader* d3dShader;
	ID3D11InputLayout* inputLayout;
};

struct PixelShader : public Resource
{
	PixelShader();
	virtual ~PixelShader() override;
	virtual void Release() override;

	ID3D11PixelShader* d3dShader;
};