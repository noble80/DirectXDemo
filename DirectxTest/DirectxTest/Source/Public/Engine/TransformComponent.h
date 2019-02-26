#pragma once
#include "Component.h"
class TransformComponent :
	public Component
{
public:
	TransformComponent();
	virtual ~TransformComponent();

	inline void SetPosition(const Vector4& pos)
	{
		transform.pos = pos;
	};

	inline Vector4 GetPosition() const
	{
		return transform.pos;
	};

	inline void SetScale(const Vector4& scale)
	{
		transform.scale = scale;
	};

	inline Vector4 GetScale() const
	{
		return transform.scale;
	};

	inline void SetRotation(const Quaternion& rot)
	{
		transform.rot = rot;
	};

	inline const Quaternion& GetRotation() const
	{
		return transform.rot;
	};

	DirectX::XMMATRIX GetTransformMatrix();
	inline Transform GetTransform()
	{
		return transform;
	};
private:
	Transform transform;
};

