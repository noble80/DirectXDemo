#include "stdafx.h"

#include "Math\MathLibrary.h"
using namespace DirectX;


Vector4 operator*(const Vector4 & lhs, const Quaternion & rhs)
{
	return DirectX::XMVector3Rotate(lhs, rhs.__xvec);
}

float debug1 = 0.005f;
float debug2 = 0.4f;

Vector4 MathLibrary::SphericalToCartesian(float phi, float theta)
{
	float x	 =	 5.0f*sinf(phi)*sinf(theta);
	float y	 =	 5.0f*cosf(phi);
	float z	 =	-5.0f*sinf(phi)*cosf(theta);

	return XMVectorSet(x, y, z, 1.f);
}
