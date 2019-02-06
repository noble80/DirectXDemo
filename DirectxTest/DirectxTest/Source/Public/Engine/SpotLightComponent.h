#pragma once
#include "stdafx.h"
#include "Component.h"

class TransformComponent;
class CameraComponent;


class SpotLightComponent :
	public Component
{
public:
	SpotLightComponent();
	virtual ~SpotLightComponent();
	virtual void Initialize(Entity* owner) override;

	inline float GetLightIntensity() const { return m_Intensity; };
	inline void SetLightIntensity(float newI) { m_Intensity = newI; };

	inline float GetRadius() const { return m_Radius; };
	inline void SetRadius(float newDis) { m_Radius = newDis; };

	inline float GetOuterAngle() const { return m_OuterAngle; };
	inline void SetOuterAngle(float newRes) { m_OuterAngle = newRes; };

	inline float GetInnerAngle() const { return m_InnerAngle; };
	inline void SetInnerAngle(float nc) { m_InnerAngle = nc; };

	inline DirectX::XMVECTOR GetLightColor() const { return m_LightColor; };
	inline void SetLightColor(DirectX::XMVECTOR newColor) { m_LightColor = newColor; };

	Vector4 GetLightDirection() const;
	Vector4 GetPosition() const;

private:
	TransformComponent* m_Transform;
	DirectX::XMVECTOR m_LightColor;
	float m_Intensity;
	float m_Radius;
	float m_OuterAngle;
	float m_InnerAngle;
};

