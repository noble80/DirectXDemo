#pragma once
#define _USE_MATH_DEFINES
#include <DirectXMath.h>
#include "MathConstants.h"
#include "Vertex.h"
#include <algorithm>
#include <math.h>
#include <typeinfo>

struct type_infoComparator
{
	bool operator ()(const std::type_info* lhs, const std::type_info* rhs) const
	{
		return lhs->before(*rhs);
	}
};
