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
		m_Position = pos;
	};

	inline Vector4 GetPosition() const
	{
		return m_Position;
	};

	inline void SetScale(const Vector4& scale)
	{
		m_Scale = scale;
	};

	inline Vector4 GetScale() const
	{
		return m_Scale;
	};

	inline void SetRotation(const Quaternion& rot)
	{
		m_Rotation = rot;
	};

	inline const Quaternion& GetRotation() const
	{
		return m_Rotation;
	};

	DirectX::XMMATRIX GetTransformMatrix();
private:
	Vector4 m_Position;
	Vector4 m_Scale;
	Quaternion m_Rotation;
};

