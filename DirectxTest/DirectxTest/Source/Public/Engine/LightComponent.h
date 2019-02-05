#pragma once
#include "stdafx.h"
#include "Component.h"

class TransformComponent;
class CameraComponent;

enum class ELightType
{
	Directional, Point, Spot
};

class LightComponent :
	public Component
{
public:
	LightComponent();
	virtual ~LightComponent();
	virtual void Initialize(Entity* owner) override;

	inline ELightType GetLightType() const { return m_LightType; };
	inline void SetLightType(ELightType type) { m_LightType = type; };
	
	inline float GetLightRadius() const { return m_Radius; };
	inline void SetLightRadius(float newRadius) { m_Radius = newRadius; };
	
	inline float GetLightFalloff() const { return m_Falloff; };
	inline void SetLightFalloff(float newF) { m_Falloff = newF; };
	
	inline float GetLightIntensity() const { return m_Intensity; };
	inline void SetLightIntensity(float newI) { m_Intensity = newI; };
	
	inline float GetShadowDistance() const { return m_ShadowDistance; };
	inline void SetShadowDistance(float newDis) { m_ShadowDistance = newDis; };

	inline float GetShadowResolution() const { return m_ShadowResolution; };
	inline void SetShadowResolution(float newRes) { m_ShadowResolution = newRes; };

	inline float GetShadowNearClip() const { return m_ShadowNearClip; };
	inline void SetShadowNearClip(float nc) { m_ShadowNearClip = nc; };

	inline float GetShadowBias() const { return m_ShadowBias; };
	inline void SetShadowBias(float bias) { m_ShadowBias = bias; };

	inline float GetNormalOffset() const { return m_NormalOffset; };
	inline void SetNormalOffset(float offset) { m_NormalOffset = offset; };

	inline DirectX::XMVECTOR GetLightColor() const { return m_LightColor; };
	inline void SetLightColor(DirectX::XMFLOAT4 newColor)  { m_LightColor = DirectX::XMVectorSet(newColor.x, newColor.y, newColor.z, newColor.w); };
	
	

	Vector4 GetLightDirection() const;

	DirectX::XMMATRIX GetViewMatrix() const;
	DirectX::XMMATRIX GetOrtographicProjectionMatrix(DirectX::XMFLOAT3 min, DirectX::XMFLOAT3 max) const;


	DirectX::XMMATRIX GetLightSpaceMatrix(CameraComponent* camera) const;
private:
	TransformComponent* m_Transform;
	ELightType m_LightType;
	DirectX::XMVECTOR m_LightColor;
	float m_Intensity;
	float m_Radius;
	float m_Falloff;
	float m_ShadowDistance;
	float m_ShadowBias;
	float m_NormalOffset;
	float m_ShadowResolution;
	float m_ShadowNearClip;
};

