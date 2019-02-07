#pragma once
#include "stdafx.h"

struct GeometryBuffer;
class Renderer;

struct Material;

namespace DebugHelpers
{
	GeometryBuffer* CreateDebugSphere(Renderer* renderer, int segments);
	extern Material* DebugMat;
	extern GeometryBuffer* DebugSphere;
}