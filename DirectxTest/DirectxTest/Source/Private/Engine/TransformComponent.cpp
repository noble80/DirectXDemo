#include "stdafx.h"
#include "Engine/TransformComponent.h"

using namespace DirectX;

TransformComponent::TransformComponent()
{
	SetPosition(XMVectorZero());
	SetRotation(XMQuaternionIdentity());
}


TransformComponent::~TransformComponent()
{
}

XMMATRIX TransformComponent::GetTransformMatrix()
{
	XMMATRIX output = m_Rotation.ToRotationMatrix() * XMMatrixTranslationFromVector(m_Position);
	return output;
}