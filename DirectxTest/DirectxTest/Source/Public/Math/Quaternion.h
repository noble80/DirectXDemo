#pragma once

#include <DirectXMath.h>

struct Quaternion
{
	Quaternion() { __xvec = DirectX::XMQuaternionIdentity(); };
	Quaternion(DirectX::XMVECTOR vec);
	inline Quaternion operator*(const Quaternion& other) const
	{
		return Quaternion(DirectX::XMQuaternionMultiply(__xvec, other.__xvec));
	};
	static Quaternion FromAngles(float pitch, float yaw, float roll);
	static Quaternion FromAxisAngle(Vector4 axis, float angle);
	static Quaternion LookAt(Vector4 forward, Vector4 up);
	DirectX::XMMATRIX ToRotationMatrix() const;

	Vector4 GetUpVector() const;
	Vector4 GetRightVector() const;
	Vector4 GetForwardVector() const;

	DirectX::XMVECTOR __xvec;
};