#include "stdafx.h"
#include "Engine/TransformComponent.h"

using namespace DirectX;

TransformComponent::TransformComponent()
{
	SetPosition(XMVectorZero());
	SetRotation(XMQuaternionIdentity());
	SetScale(XMVectorSet(1.f, 1.f, 1.f, 1.f));
}


TransformComponent::~TransformComponent()
{
}

XMMATRIX TransformComponent::GetTransformMatrix()
{
	XMMATRIX output = XMMatrixScalingFromVector(m_Scale) * m_Rotation.ToRotationMatrix() * XMMatrixTranslationFromVector(m_Position);
	return output;
}