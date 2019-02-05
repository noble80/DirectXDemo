#include "stdafx.h"
#include "Renderer\Texture2D.h"

Texture2D::~Texture2D()
{
}

void Texture2D::Release()
{
	if(d3dresource)
		d3dresource->Release();
	if(shaderResourceView)
		shaderResourceView->Release();
}