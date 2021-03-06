#pragma once

#include "Math/Vertex.h"

namespace MeshImporter
{
	struct Mesh
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		std::string name;
	};
}