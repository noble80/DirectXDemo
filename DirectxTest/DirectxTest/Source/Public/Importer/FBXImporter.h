#pragma once
#include "stdafx.h"
#include "Mesh.h"

typedef uint16_t FBX_IMPORT_FLAG;


#define FBX_IMPORT_FLAG_SWAP_YZ 0b1


namespace FBXImporter
{

	bool ImportModelFromFile(std::string path, std::vector<Mesh>& meshes, float scale, FBX_IMPORT_FLAG flags);

};