#include "stdafx.h"
#include "Math/Quaternion.h"

using namespace DirectX;

Quaternion::Quaternion(DirectX::XMVECTOR vec)
{
	__xvec = vec;
}

Quaternion Quaternion::FromAngles(float pitch, float yaw, float roll)
{


	return Quaternion(XMQuaternionRotationRollPitchYaw(XMConvertToRadians(pitch), XMConvertToRadians(yaw), XMConvertToRadians(roll)));
}

Quaternion Quaternion::FromAxisAngle(Vector4 axis, float angle)
{
	return Quaternion(XMQuaternionRotationAxis(axis, XMConvertToRadians(angle)));
}

Quaternion Quaternion::LookAt(Vector4 forward, Vector4 up)
{
	MathLibrary::Orthonormalize(forward, up);

	Vector4 right = XMVector3Cross(up, forward);
	Quaternion output;
	output.__xvec.m128_f32[3] = sqrtf(1.0f + right.m128_f32[0] + up.m128_f32[1] + forward.m128_f32[2]) * 0.5f;

	float reciprocal = 1.0f / (4.0f * output.__xvec.m128_f32[3]);

	output.__xvec.m128_f32[0] = (up.m128_f32[2] - forward.m128_f32[1]) * reciprocal;
	output.__xvec.m128_f32[1] = (forward.m128_f32[0] - right.m128_f32[2]) * reciprocal;
	output.__xvec.m128_f32[2] = (right.m128_f32[1] - up.m128_f32[0]) * reciprocal;

	return output;
}

DirectX::XMMATRIX Quaternion::ToRotationMatrix() const
{
	return XMMatrixRotationQuaternion(__xvec);
}

Vector4 Quaternion::GetUpVector() const
{
	return VectorConstants::Up * *this;
}

Vector4 Quaternion::GetRightVector() const
{
	return VectorConstants::Right * *this;
}

Vector4 Quaternion::GetForwardVector() const
{
	return VectorConstants::Forward * *this;
}
