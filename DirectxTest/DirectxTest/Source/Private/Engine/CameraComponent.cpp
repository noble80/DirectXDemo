
#include "stdafx.h"
#include "Engine/CameraComponent.h"
#include "Engine\TransformComponent.h"

using namespace DirectX;

CameraComponent::CameraComponent()
{
}


CameraComponent::~CameraComponent()
{

}

void CameraComponent::Initialize(Entity * owner)
{
	Component::Initialize(owner);
	m_Transform = m_Owner->GetComponent<TransformComponent>();
}

void CameraComponent::SetProjectionMatrix(float FOV, float aspectRatio, float nearZ, float farZ)
{
	float verticalFOV = XMConvertToRadians(FOV/aspectRatio);
	m_ProjectionMatrix = XMMatrixPerspectiveFovLH(verticalFOV, aspectRatio, nearZ, farZ);
	m_FOV = FOV;
	m_AspectRatio = aspectRatio;
	m_NearZ = nearZ;
	m_FarZ = farZ;
}

Vector4 CameraComponent::GetCameraPosition() const
{
	return m_Transform->GetPosition();
}

DirectX::XMMATRIX CameraComponent::GetViewProjectionMatrix() const
{
	return GetViewMatrix() * GetOrtographicProjectionMatrix();
}

DirectX::XMMATRIX CameraComponent::GetViewMatrix() const
{
	return XMMatrixInverse(nullptr, m_Transform->GetTransformMatrix());
}

DirectX::XMMATRIX CameraComponent::GetCameraMatrix() const
{
	return m_Transform->GetTransformMatrix();
}
