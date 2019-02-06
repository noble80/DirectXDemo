#include "stdafx.h"
#include "Engine/DirectionalLightComponent.h"

#include "Engine/Entity.h"
#include "Engine/TransformComponent.h"
#include "Engine/CameraComponent.h"

using namespace DirectX;
using namespace MathConstants;
using namespace VectorConstants;

DirectionalLightComponent::DirectionalLightComponent()
{
	SetLightColor(XMVectorSet(1.f, 1.f, 1.f, 1.f));
	SetLightIntensity(1.0f);
	SetShadowDistance(100.f);
	SetShadowResolution(4096);
	SetShadowNearClip(0.01f);
	SetShadowBias(0.01f);
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

DirectX::XMMATRIX DirectionalLightComponent::GetViewMatrix() const
{
	Vector4 pos = -GetLightDirection()*(GetShadowDistance() - GetShadowNearClip())*0.5f;	// away from the origin along the direction vector 
	XMMATRIX view = XMMatrixLookAtLH(pos, Zero, Up);
	return view;
}

DirectX::XMMATRIX DirectionalLightComponent::GetOrtographicProjectionMatrix(XMFLOAT3 min, XMFLOAT3 max) const
{
	//min = XMFLOAT3(-30.f, -30.f, 0.f);
	//max  = XMFLOAT3(30.f, 30.f, 0.f);

	XMMATRIX proj = XMMatrixOrthographicOffCenterLH(min.x, max.x, min.y, max.y, GetShadowNearClip(), GetShadowDistance());
	return proj;
}

DirectX::XMMATRIX DirectionalLightComponent::GetLightSpaceMatrix(CameraComponent* camera) const
{
	float verticalFOV = XMConvertToRadians(camera->GetFOV());
	XMMATRIX viewProj = camera->GetViewProjectionMatrix();
	Vector4 min, max;
	XMMATRIX inverse = XMMatrixInverse(nullptr, viewProj);

	Vector4 boundingBox[8];
	boundingBox[0] = XMVectorSet(-1.0f, -1.0f, 0, 1.0f);
	boundingBox[1] = XMVectorSet(1.0f, -1.0f, 0, 1.0f);
	boundingBox[2] = XMVectorSet(-1.0f, 1.0f, 0, 1.0f);
	boundingBox[3] = XMVectorSet(1.0f, 1.0f, 0, 1.0f);
	boundingBox[4] = XMVectorSet(-1.0f, -1.0f, 1.0f, 1.0f);
	boundingBox[5] = XMVectorSet(1.0f, -1.0f, 1.0f, 1.0f);
	boundingBox[6] = XMVectorSet(-1.0f, 1.0f, 1.0f, 1.0f);
	boundingBox[7] = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);

	for(int i = 0; i < 8; i++)
	{
		boundingBox[i] = XMVector3TransformCoord(boundingBox[i], inverse);
	}

	Vector4 center = boundingBox[0];
	for(int i = 1; i < 8; i++)
	{
		center += boundingBox[i];
	}
	center = center / 8.f;
	XMMATRIX view = GetViewMatrix();

	Vector4 vec = XMVector3TransformCoord(boundingBox[0], view);
	min = vec;
	max = vec;

	for(int i = 1; i < 8; i++)
	{
		vec = XMVector3TransformCoord(boundingBox[i], view);

		min = XMVectorMin(min, vec);
		max = XMVectorMax(max, vec);
	}


	float fNormalizeByBufferSize = (1.0f / m_ShadowResolution);
	XMVECTOR vNormalizeByBufferSize = XMVectorSet(fNormalizeByBufferSize, fNormalizeByBufferSize, fNormalizeByBufferSize, 0.0f);
	XMVECTOR vWorldUnitsPerTexel = max - min;
	vWorldUnitsPerTexel *= vNormalizeByBufferSize;

	min /= vWorldUnitsPerTexel;
	min = XMVectorFloor(min);
	min *= vWorldUnitsPerTexel;

	max /= vWorldUnitsPerTexel;
	max = XMVectorFloor(max);
	max *= vWorldUnitsPerTexel;

	XMFLOAT3 a, b;
	XMStoreFloat3(&a, min);
	XMStoreFloat3(&b, max);

	XMMATRIX proj = GetOrtographicProjectionMatrix(a, b);

	return view * proj;
}
