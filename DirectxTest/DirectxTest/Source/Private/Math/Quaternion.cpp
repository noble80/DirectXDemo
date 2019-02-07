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
