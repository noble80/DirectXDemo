#include "stdafx.h"
#include "Engine/ModelComponent.h"
#include "Engine/TransformComponent.h"
#include "Engine/Entity.h"

ModelComponent::~ModelComponent()
{
}

void ModelComponent::Initialize(Entity * owner)
{
	Component::Initialize(owner);
	m_Transform = m_Owner->GetComponent<TransformComponent>();
}

DirectX::XMMATRIX ModelComponent::GetTransformMatrix()
{
	return m_Transform->GetTransformMatrix();
}
