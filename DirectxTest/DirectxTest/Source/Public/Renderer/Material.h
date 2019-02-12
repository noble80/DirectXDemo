#pragma once
#include "ShaderContainers.h"
#include "SurfaceParameters.h"

struct Texture2D;

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
			Texture2D* detailsMap;
			Texture2D* normalMap;
			union
			{
				Texture2D* reflectionMap;
				Texture2D* IBLDiffuse;
			};
			Texture2D* IBLSpecular;
			Texture2D* IBLIntegration;
		};

		Texture2D* textures[6];
	};

	Material();
	virtual ~Material() override;
	virtual void Release() override;
};