#include "stdafx.h"
#include "Renderer\Texture2D.h"

Texture2D::~Texture2D()
{
}

void Texture2D::Release()
{
	if(texture)
		texture->Release();
	if(resourceView)
		resourceView->Release();
}