#pragma once

#include "Texture2D.h"

struct RenderTexture2D : public Texture2D
{
	ID3D11RenderTargetView* renderTargetView;
	ID3D11DepthStencilView* depthStencilView;
	ID3D11ShaderResourceView* depthView;

	Vector2 dimensions;

	RenderTexture2D();
	virtual ~RenderTexture2D() override;
	virtual void Release() override;
};

struct RenderTexture2DAllMips : public Resource
{
	ID3D11Texture2D* texture;
	std::vector<ID3D11RenderTargetView*> RTVs;
	ID3D11ShaderResourceView* resourceView;

	RenderTexture2DAllMips();
	virtual ~RenderTexture2DAllMips() override;
	virtual void Release() override;
};

struct DepthTexture2D : public Resource
{
	ID3D11Texture2D* texture;
	std::vector<ID3D11DepthStencilView*> DSVs;
	ID3D11ShaderResourceView* resourceView;

	DepthTexture2D();
	virtual ~DepthTexture2D() override;
	virtual void Release() override;
};

