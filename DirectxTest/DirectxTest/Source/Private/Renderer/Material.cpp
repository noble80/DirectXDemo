#include "stdafx.h"
#include "Renderer\Material.h"

Material::Material()
{
	diffuseMap = nullptr;
	specularMap = nullptr;
	glossinessMap = nullptr;
	normalMap = nullptr;
	AOMap = nullptr;
	reflectionMap = nullptr;
}

Material::~Material()
{
}

void Material::Release()
{

}