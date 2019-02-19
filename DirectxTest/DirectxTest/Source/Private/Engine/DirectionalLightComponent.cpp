#include "stdafx.h"
#include "Engine/DirectionalLightComponent.h"

#include "Engine/Entity.h"
#include "Engine/TransformComponent.h"
#include "Engine/CameraComponent.h"

#include "Renderer\Renderer.h"

using namespace DirectX;
using namespace MathConstants;
using namespace VectorConstants;

DirectionalLightComponent::DirectionalLightComponent()
{
	SetLightColor(XMVectorSet(1.f, 1.f, 1.f, 1.f));
	SetLightIntensity(1.0f);
	SetShadowDistance(100.f);
	SetShadowResolution(2048);
	SetShadowNearClip(0.01f);
	SetShadowBias(0.004f);
	SetNormalOffset(0.4f);
}

DirectionalLightComponent::~DirectionalLightComponent()
{

}

void DirectionalLightComponent::Initialize(Entity * owner)
{
	Component::Initialize(owner);
	m_Transform = m_Owner->GetComponent<TransformComponent>();
}

Vector4 DirectionalLightComponent::GetLightDirection() const
{
	return Forward * m_Transform->GetRotation();
}

XMMATRIX DirectionalLightComponent::GetGlobalLightMatrix(CameraComponent* camera) const
{
	XMMATRIX cameraViewProj = camera->GetViewProjectionMatrix();
	XMMATRIX inverseCamera = XMMatrixInverse(nullptr, cameraViewProj);

	Vector4 boundingBox[8];
	boundingBox[0] = XMVectorSet(1.0f, 1.0f, 0, 1.0f);
	boundingBox[1] = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	boundingBox[2] = XMVectorSet(1.0f, -1.0f, 0, 1.0f);
	boundingBox[3] = XMVectorSet(1.0f, -1.0f, 1.0f, 1.0f);
	boundingBox[4] = XMVectorSet(-1.0f, 1.0f, 0, 1.0f);
	boundingBox[5] = XMVectorSet(-1.0f, 1.0f, 1.0f, 1.0f);
	boundingBox[6] = XMVectorSet(-1.0f, -1.0f, 0, 1.0f);
	boundingBox[7] = XMVectorSet(-1.0f, -1.0f, 1.0f, 1.0f);

	Vector4 vRadius = XMVectorZero();
	Vector4 cameraPos = camera->GetCameraPosition();
	Vector4 center = XMVectorZero();
	for(int i = 0; i < 8; i++)
	{
		boundingBox[i] = XMVector3TransformCoord(boundingBox[i], inverseCamera);
		center += boundingBox[i];
	}
	center = center / 8.f;

	XMVECTOR lightPos = center - GetLightDirection()*0.5f;
	XMVECTOR lightLookAt = center;
	XMMATRIX view = XMMatrixLookAtLH(lightPos, lightLookAt, Up);
	XMMATRIX proj = XMMatrixOrthographicOffCenterLH(-0.5f, 0.5f, -0.5f, 0.5f, 0.0f, 1.0f);
	XMMATRIX viewProj = view * proj;

	return viewProj;
}

void DirectionalLightComponent::GetLightSpaceMatrices(CameraComponent* camera, CascadeShadows* cascades) const
{
	int cascadeCount = cascades->cascadeCount;
	cascades->cascadeMatrices.resize(cascadeCount);
	cascades->cascadeSplits.resize(cascadeCount);

	std::vector<float> cascadeSplits;
	cascadeSplits.resize(cascadeCount);

	float lambda = 0.5f; //Blend factor between logarithmic and linear distributions

	float nearZ = camera->GetNearZ();
	float farZ = camera->GetFarZ();
	float rangeZ = farZ - nearZ;

	float minZ = nearZ; // TODO: add parameters for minimum and maximum cascade distances
	float maxZ = farZ;

	float range = maxZ - minZ;
	float ratio = maxZ / minZ;

	for(int i = 0; i < cascadeCount; ++i)
	{
		float alpha = (i + 1) / static_cast<float>(cascadeCount);
		float linear = minZ + range * alpha;
		float logarithmic = minZ * std::pow(ratio, alpha);
		float distance = lambda * (logarithmic - linear) + linear;
		cascadeSplits[i] = (distance - nearZ) / rangeZ;
	}


	XMMATRIX cameraViewProj = camera->GetViewProjectionMatrix();
	XMMATRIX inverseCamera = XMMatrixInverse(nullptr, cameraViewProj);

	Vector4 cameraFrustrumWS[8];
	cameraFrustrumWS[0] = XMVectorSet(-1.0f, 1.0f, 0.0f, 1.0f);
	cameraFrustrumWS[1] = XMVectorSet(1.0f, 1.0f, 0.0f, 1.0f);
	cameraFrustrumWS[2] = XMVectorSet(1.0f, -1.0f, 0.0f, 1.0f);
	cameraFrustrumWS[3] = XMVectorSet(-1.0f, -1.0f, 0.0f, 1.0f);
	cameraFrustrumWS[4] = XMVectorSet(-1.0f, 1.0f, 1.0f, 1.0f);
	cameraFrustrumWS[5] = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	cameraFrustrumWS[6] = XMVectorSet(1.0f, -1.0f, 1.0f, 1.0f);
	cameraFrustrumWS[7] = XMVectorSet(-1.0f, -1.0f, 1.0f, 1.0f);
	for(int i = 0; i < 8; i++)
	{
		cameraFrustrumWS[i] = XMVector3TransformCoord(cameraFrustrumWS[i], inverseCamera);
	}

	for(int i = 0; i < cascadeCount; ++i)
	{
		float prevSplit = i == 0 ? 0.f : cascadeSplits[i - 1];
		float currSplit = cascadeSplits[i];

		cascades->cascadeSplits[i] = nearZ + currSplit * rangeZ;

		Vector4 vRadius = XMVectorZero();
		Vector4 cameraPos = camera->GetCameraPosition();
		Vector4 center = XMVectorZero();

		Vector4 splitFrustrumWS[8];

		// Get current split's corners
		for(int j = 0; j < 4; ++j)
		{
			Vector4 cornerToCorner = cameraFrustrumWS[j + 4] - cameraFrustrumWS[j];
			Vector4 nearCorner = cornerToCorner * prevSplit;
			Vector4 farCorner = cornerToCorner * currSplit;
			splitFrustrumWS[j + 4] = cameraFrustrumWS[j] + farCorner;
			splitFrustrumWS[j] = cameraFrustrumWS[j] + nearCorner;
		}

		for(int j = 0; j < 8; ++j)
		{
			center += splitFrustrumWS[j];
		}
		center /= 8.f;

		for(int j = 0; j < 8; j++)
		{
			vRadius = XMVectorMax(XMVector3Length(splitFrustrumWS[j] - center), vRadius);
		}
		XMVectorSetW(vRadius, 1.f);
		float radius = XMVectorGetX(vRadius);

		// Not ceiling cascades gives me problems. have to fix
		radius = std::ceil(radius);
		XMVECTOR lightPos = center - GetLightDirection()*radius;
		XMVECTOR lightLookAt = center;
		XMMATRIX view = XMMatrixLookAtLH(lightPos, lightLookAt, Up);
		XMMATRIX proj = XMMatrixOrthographicOffCenterLH(-radius, radius, -radius, radius, 0.0f, radius*2.f);
		XMMATRIX viewProj = view * proj;

		// Stabilize shadow shimmering
		XMMATRIX& shadowMatrix = viewProj;
		XMVECTOR shadowOrigin = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		shadowOrigin = XMVector3Transform(shadowOrigin, shadowMatrix);
		shadowOrigin = XMVectorScale(shadowOrigin, m_ShadowResolution * 0.5f);

		XMVECTOR roundedOrigin = XMVectorFloor(shadowOrigin);
		XMVECTOR roundOffset = XMVectorSubtract(shadowOrigin, roundedOrigin);
		roundOffset = XMVectorScale(roundOffset, 2.0f / m_ShadowResolution);
		roundOffset = XMVectorSetZ(roundOffset, 0.0f);
		roundOffset = XMVectorSetW(roundOffset, 0.0f);

		viewProj = viewProj * XMMatrixTranslationFromVector(-roundOffset);

		cascades->cascadeMatrices[i] = viewProj;
	}
}
