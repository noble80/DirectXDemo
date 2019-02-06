#include "stdafx.h"

#include "Engine\SpotLightComponent.h"

#include "Engine/Entity.h"
#include "Engine/TransformComponent.h"
#include "Engine/CameraComponent.h"

using namespace DirectX;
using namespace MathConstants;
using namespace VectorConstants;

SpotLightComponent::SpotLightComponent()
{
	SetLightColor(XMVectorSet(1.f, 1.f, 1.f, 1.f));
	SetRadius(5.0f);
	SetLightIntensity(1.0f);
	SetInnerAngle(40.f);
	SetOuterAngle(50.f);
}

SpotLightComponent::~SpotLightComponent()
{
}

void SpotLightComponent::Initialize(Entity * owner)
{
	Component::Initialize(owner);
	m_Transform = m_Owner->GetComponent<TransformComponent>();
}

Vector4 SpotLightComponent::GetLightDirection() const
{
	return Forward * m_Transform->GetRotation();
}

Vector4 SpotLightComponent::GetPosition() const
{
	return m_Transform->GetPosition();
}
