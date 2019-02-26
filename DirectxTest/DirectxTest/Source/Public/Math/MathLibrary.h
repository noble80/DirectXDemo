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

struct Transform
{
	Vector4 pos;
	Vector4 scale;
	Quaternion rot;

	DirectX::XMMATRIX Matrix() const { return DirectX::XMMatrixScalingFromVector(scale) * rot.ToRotationMatrix() * DirectX::XMMatrixTranslationFromVector(pos); }
};

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

	float RandomFloatInRange(float min, float max);
	Vector4 RandomPointInCircle(Vector4 center, float radius);
	void Orthonormalize(Vector4 normal, Vector4& tangent);
}

Vector4 operator*(const Vector4& lhs, const Quaternion& rhs);

extern float debug1;
extern float debug2;