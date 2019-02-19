#pragma once
#include "stdafx.h"
#include "Component.h"

class TransformComponent;
class CameraComponent;

struct CascadeShadows;

class DirectionalLightComponent :
	public Component
{
public:
	DirectionalLightComponent();
	virtual ~DirectionalLightComponent();
	virtual void Initialize(Entity* owner) override;
	
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
	inline void SetLightColor(DirectX::XMVECTOR newColor)  { m_LightColor = newColor; };
	
	

	Vector4 GetLightDirection() const;



	void GetLightSpaceMatrices(CameraComponent* camera, CascadeShadows* cascade) const;
private:

	DirectX::XMMATRIX GetGlobalLightMatrix(CameraComponent* camera) const;

	TransformComponent* m_Transform;
	DirectX::XMVECTOR m_LightColor;
	float m_Intensity;
	float m_ShadowDistance;
	float m_ShadowBias;
	float m_ShadowResolution;
	float m_NormalOffset;
	float m_ShadowNearClip;
};

