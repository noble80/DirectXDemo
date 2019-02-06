#pragma once
#include <DirectXMath.h>
#include "MathConstants.h"
#include "Vertex.h"
#include "Vector4.h"
#include "Vector2.h"
#include "Quaternion.h"
#include <algorithm>

struct type_infoComparator
{
	bool operator ()(const std::type_info* lhs, const std::type_info* rhs) const
	{
		return lhs->before(*rhs);
	}
};

Vector4 operator*(const Vector4& lhs, const Quaternion& rhs);

extern float debug1;
extern float debug2;