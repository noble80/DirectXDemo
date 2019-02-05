#pragma once

#include "GraphicsStructures.h"

struct Material;
struct GeometryBuffer;

struct Mesh : public Resource
{
	virtual ~Mesh() override;
	virtual void Release() override;

	Material* material;
	GeometryBuffer* geometry;
};