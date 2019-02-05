#pragma once

#include <DirectXMath.h>

struct Quaternion
{
	Quaternion() {};
	Quaternion(DirectX::XMVECTOR vec);
	inline Quaternion operator*(const Quaternion& other) const
	{
		return Quaternion(DirectX::XMQuaternionMultiply(__xvec, other.__xvec));
	};
	static Quaternion FromAngles(float pitch, float yaw, float roll);
	static Quaternion FromAxisAngle(Vector4 axis, float angle);
	DirectX::XMMATRIX ToRotationMatrix() const;

	Vector4 GetUpVector() const;
	Vector4 GetRightVector() const;
	Vector4 GetForwardVector() const;

	DirectX::XMVECTOR __xvec;
};