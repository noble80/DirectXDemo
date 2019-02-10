#pragma once
#include "Texture2D.h"
#include "ShaderContainers.h"
#include "SurfaceParameters.h"

struct Material : Resource
{
public:
	VertexShader* vertexShader;
	PixelShader*	pixelShader;
	SurfaceProperties surfaceParameters;

	union
	{
		struct
		{
			Texture2D* diffuseMap;
			Texture2D* specularMap;
			Texture2D* glossinessMap;
			Texture2D* normalMap;
			Texture2D* AOMap;
			Texture2D* reflectionMap;
		};

		Texture2D* textures[6];
	};

	Material();
	virtual ~Material() override;
	virtual void Release() override;
};