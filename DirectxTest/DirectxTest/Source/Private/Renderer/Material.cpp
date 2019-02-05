#include "stdafx.h"
#include "Renderer\Material.h"

Material::Material()
{
	vertexShader = nullptr;
	pixelShader = nullptr;
	inputLayout = nullptr;
}

Material::~Material()
{
}

void Material::Release()
{
	if(vertexShader)
		vertexShader->Release();
	if(pixelShader)
		pixelShader->Release();
	if(inputLayout)
		inputLayout->Release();
}