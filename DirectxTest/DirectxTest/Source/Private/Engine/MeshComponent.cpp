#include "stdafx.h"
#include "Engine\MeshComponent.h"
#include "Engine/TransformComponent.h"
#include "Engine/Entity.h"

MeshComponent::~MeshComponent()
{
}

void MeshComponent::Initialize(Entity * owner)
{
	Component::Initialize(owner);
	m_Transform = m_Owner->GetComponent<TransformComponent>();
}

DirectX::XMMATRIX MeshComponent::GetTransformMatrix()
{
	return m_Transform->GetTransformMatrix();
}
