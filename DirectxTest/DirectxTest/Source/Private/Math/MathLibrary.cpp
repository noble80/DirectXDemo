#include "stdafx.h"

#include "Math\MathLibrary.h"
#include <random>
using namespace DirectX;


Vector4 operator*(const Vector4 & lhs, const Quaternion & rhs)
{
	return DirectX::XMVector3Rotate(lhs, rhs.__xvec);
}

float debug1 = 0.005f;
float debug2 = 0.4f;

Vector4 MathLibrary::SphericalToCartesian(float phi, float theta)
{
	float x = 5.0f*sinf(phi)*sinf(theta);
	float y = 5.0f*cosf(phi);
	float z = -5.0f*sinf(phi)*cosf(theta);

	return XMVectorSet(x, y, z, 1.f);
}

std::random_device rd;
std::default_random_engine generator(rd());
std::uniform_real_distribution<double> distribution(0.0, 1.0);
float MathLibrary::RandomFloatInRange(float min, float max)
{
	return distribution(generator)*(max - min) + min;
}
Vector4 MathLibrary::RandomPointInCircle(Vector4 center, float radius)
{
	double a = distribution(generator) * 2 * M_PI;
	double r = radius * sqrt(distribution(generator));

	// If you need it in Cartesian coordinates
	double x = r * cos(a);
	double y = r * sin(a);

	return center + XMVectorSet(x, 0.f, y, 0.f);
}

void MathLibrary::Orthonormalize(Vector4 normal, Vector4& tangent)
{
	normal = XMVector3Normalize(normal);
	Vector4 proj;
	proj = XMVectorMultiply(normal, XMVector3Dot(tangent, normal));

	tangent = tangent - proj;

	tangent = XMVector3Normalize(tangent);
}
