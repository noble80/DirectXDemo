#include "stdafx.h"

#include "Renderer\ShaderContainers.h"

VertexShader::VertexShader()
{
	d3dShader = nullptr;
	inputLayout = nullptr;
}

VertexShader::~VertexShader()
{
}

void VertexShader::Release()
{
	if(d3dShader)
		d3dShader->Release();

	if(inputLayout)
		inputLayout->Release();
}

PixelShader::PixelShader()
{
	d3dShader = nullptr;
}

PixelShader::~PixelShader()
{
}

void PixelShader::Release()
{
	if(d3dShader)
		d3dShader->Release();
}
