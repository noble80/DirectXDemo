#include "stdafx.h"
#include "Renderer/RenderTexture2D.h"

RenderTexture2D::RenderTexture2D()
{
	resourceView = nullptr;
	texture = nullptr;
	renderTargetView = nullptr;
	depthView = nullptr;
	depthStencilView = nullptr;
}

RenderTexture2D::~RenderTexture2D()
{
}

void RenderTexture2D::Release()
{
	if(depthStencilView)
		depthStencilView->Release();

	if(renderTargetView)
		renderTargetView->Release();

	if(resourceView)
		resourceView->Release();

	if(texture)
		texture->Release();

	if(depthView)
		depthView->Release();
}

DepthTexture2D::DepthTexture2D()
{
	resourceView = nullptr;
	texture = nullptr;
}

DepthTexture2D::~DepthTexture2D()
{
}

void DepthTexture2D::Release()
{
	for(int i = 0; i < DSVs.size(); ++i)
	{
		DSVs[i]->Release();
	}

	if(resourceView)
		resourceView->Release();

	if(texture)
		texture->Release();
}

RenderTexture2DAllMips::RenderTexture2DAllMips()
{
	resourceView = nullptr;
	texture = nullptr;
}

RenderTexture2DAllMips::~RenderTexture2DAllMips()
{
}

void RenderTexture2DAllMips::Release()
{
	for(int i = 0; i < RTVs.size(); ++i)
	{
		RTVs[i]->Release();
	}

	if(resourceView)
		resourceView->Release();

	if(texture)
		texture->Release();
}
