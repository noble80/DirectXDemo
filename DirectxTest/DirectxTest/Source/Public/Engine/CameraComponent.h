#pragma once

#include "Entity.h"

#include "stdafx.h"

class TransformComponent;

class CameraComponent : public Component
{
public:
	CameraComponent();
	~CameraComponent();

	virtual void Initialize(Entity* owner) override;

	void SetProjectionMatrix(float FOV, Vector2 screenDimensions, float nearZ, float farZ);
	inline const DirectX::XMMATRIX& GetProjectionMatrix() const
	{
		return m_ProjectionMatrix;
	};
	void RebuildProjectionMatrix();
	void UpdateFOV(float FOV);
	void UpdateAspectRatio(Vector2 dimensions);

	inline float GetFOV() const { return m_FOV; };

	inline float GetNearZ() const { return m_NearZ; };

	inline float GetFarZ() const { return m_FarZ; };

	inline float GetAspectRatio() const { return m_AspectRatio; };

	Vector4 GetCameraPosition() const;

	DirectX::XMMATRIX GetViewProjectionMatrix() const;
	DirectX::XMMATRIX GetViewMatrix() const;
	DirectX::XMMATRIX GetCameraMatrix() const;
private:
	float m_NearZ;
	float m_FarZ;
	float m_AspectRatio;
	float m_FOV;
	DirectX::XMMATRIX m_ProjectionMatrix;
	TransformComponent* m_Transform;
};

