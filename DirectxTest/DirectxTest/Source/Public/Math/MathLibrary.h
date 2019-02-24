#pragma once
#define _USE_MATH_DEFINES
#include <DirectXMath.h>
#include "MathConstants.h"
#include "Vertex.h"
#include "Vector4.h"
#include "Vector2.h"
#include "Quaternion.h"
#include <algorithm>
#include <math.h>

struct type_infoComparator
{
	bool operator ()(const std::type_info* lhs, const std::type_info* rhs) const
	{
		return lhs->before(*rhs);
	}
};

namespace MathLibrary
{
	Vector4 SphericalToCartesian(float phi, float theta);
	template <typename T>
	inline T lerp(T v0, T v1, T t)
	{
		return fma(t, v1, fma(-t, v0, v0));
	}
}

Vector4 operator*(const Vector4& lhs, const Quaternion& rhs);

extern float debug1;
extern float debug2;