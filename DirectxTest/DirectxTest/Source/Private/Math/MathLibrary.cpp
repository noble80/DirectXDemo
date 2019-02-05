#include "stdafx.h"

#include "Math\MathLibrary.h"
using namespace DirectX;


Vector4 operator*(const Vector4 & lhs, const Quaternion & rhs)
{
	return DirectX::XMVector3Rotate(lhs, rhs.__xvec);
}

float debug1 = 0.005f;
float debug2 = 0.4f;