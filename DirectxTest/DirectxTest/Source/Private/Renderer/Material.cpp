#include "stdafx.h"
#include "Renderer\Material.h"

Material::Material()
{
	diffuseMap = nullptr;
	detailsMap = nullptr;
	normalMap = nullptr;
	reflectionMap = nullptr;
	IBLSpecular = nullptr;
	IBLIntegration = nullptr;
}

Material::~Material()
{
}

void Material::Release()
{

}