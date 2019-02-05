#include "stdafx.h"
#include "Renderer/RenderTexture2D.h"

RenderTexture2D::RenderTexture2D() : Texture2D()
{
	d3dtexture = nullptr;
	renderTargetView = nullptr;
	d3dstencil = nullptr;
	depthStencilView = nullptr;
}

RenderTexture2D::~RenderTexture2D()
{
}

void RenderTexture2D::Release()
{
	Texture2D::Release();

	if(d3dtexture)
		d3dtexture->Release();
	if(renderTargetView)
		renderTargetView->Release();
	if(d3dstencil)
		d3dstencil->Release();
	if(depthStencilView)
		depthStencilView->Release();
}