#include "stdafx.h"
#include "Engine\PointLightComponent.h"

#include "Engine/Entity.h"
#include "Engine/TransformComponent.h"
#include "Engine/CameraComponent.h"

using namespace DirectX;
using namespace MathConstants;
using namespace VectorConstants;

PointLightComponent::PointLightComponent()
{
	SetLightColor(XMVectorSet(1.f, 1.f, 1.f, 1.f));
	SetRadius(5.0f);
	SetLightIntensity(1.0f);
}

PointLightComponent::~PointLightComponent()
{
}

void PointLightComponent::Initialize(Entity * owner)
{
	Component::Initialize(owner);
	m_Transform = m_Owner->GetComponent<TransformComponent>();
}

Vector4 PointLightComponent::GetPosition() const
{
	return m_Transform->GetPosition();
}
