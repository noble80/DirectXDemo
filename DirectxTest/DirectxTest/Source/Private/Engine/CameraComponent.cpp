
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

void CameraComponent::SetProjectionMatrix(float FOV, Vector2 dimensions, float nearZ, float farZ)
{
	m_FOV = FOV;
	m_AspectRatio = dimensions.x / dimensions.y;;
	m_NearZ = nearZ;
	m_FarZ = farZ;
	RebuildProjectionMatrix();
}

void CameraComponent::RebuildProjectionMatrix()
{
	float verticalFOV = XMConvertToRadians(m_FOV / m_AspectRatio);
	m_ProjectionMatrix = XMMatrixPerspectiveFovLH(verticalFOV, m_AspectRatio, m_NearZ, m_FarZ);
}

void CameraComponent::UpdateFOV(float FOV)
{
	m_FOV = FOV;
	RebuildProjectionMatrix();
}

void CameraComponent::UpdateAspectRatio(Vector2 dimensions)
{
	m_AspectRatio = dimensions.x / dimensions.y;
	RebuildProjectionMatrix();
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
