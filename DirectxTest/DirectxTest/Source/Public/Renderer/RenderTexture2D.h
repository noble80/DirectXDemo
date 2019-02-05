#pragma once
#include "Texture2D.h"

struct RenderTexture2D : public Texture2D
{

	ID3D11Texture2D* d3dtexture;
	ID3D11RenderTargetView* renderTargetView;
	ID3D11Texture2D* d3dstencil;
	ID3D11DepthStencilView* depthStencilView;

	RenderTexture2D();
	virtual ~RenderTexture2D() override;
	virtual void Release() override;
};