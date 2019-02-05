#pragma once
#include "Texture2D.h"

struct Material : Resource
{
public:
	ID3D11VertexShader* vertexShader;
	ID3D11PixelShader*	pixelShader;
	ID3D11InputLayout* inputLayout;
	std::vector<Texture2D*> textures;

	Material();
	virtual ~Material() override;
	virtual void Release() override;
};