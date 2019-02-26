#include "stdafx.h"
#include "Engine\InstancedMeshComponent.h"
#include "Engine/TransformComponent.h"
#include "Engine/Entity.h"

#include "Renderer\Renderer.h"

using namespace DirectX;
using namespace MathLibrary;

InstancedMeshComponent::InstancedMeshComponent()
{
}

void InstancedMeshComponent::CreateInstanceBuffer(Renderer* renderer)
{
	constantBuffer = renderer->CreateConstantBuffer(sizeof(CInstanceInfo), GetOwner()->GetName() + "Instanced");
}


InstancedMeshComponent::~InstancedMeshComponent()
{
}

void InstancedMeshComponent::Initialize(Entity * owner)
{
	Component::Initialize(owner);
}

void InstancedMeshComponent::AddInstance(Vector4 pos, Quaternion rot, Vector4 scale)
{
	Transform t;
	t.pos = pos;
	t.rot = rot;
	t.scale = scale;
	AddInstance(t);

}

void InstancedMeshComponent::AddInstance(const Transform & transform)
{
	m_Transforms.push_back(transform);
}

std::vector<Transform>* InstancedMeshComponent::GetTransformMatrices()
{
	return &m_Transforms;
}

std::vector<Vector4> InstancedMeshComponent::GetPositions()
{
	std::vector<Vector4> output;
	output.resize(m_Transforms.size());

	for(int i = 0; i < m_Transforms.size(); ++i)
	{
		output[i] = m_Transforms[i].pos;
	}

	return output;
}

void InstancedMeshComponent::SetInstanceCount(int n)
{
	m_Transforms.resize(n);
}

void InstancedMeshComponent::RandomizeInstanceTransforms(float radius, float pitch, float yaw, float roll, Vector2 scale)
{
	m_Radius = radius;
	m_Pitch = pitch;
	m_Yaw = yaw;
	m_Roll = roll;
	m_Scale = scale;

	RandomizeInstanceTransforms();
}

void InstancedMeshComponent::RandomizeInstanceTransforms()
{
	for(Transform& t : m_Transforms)
	{
		t.pos = MathLibrary::RandomPointInCircle(VectorConstants::Zero, m_Radius);
		t.rot = Quaternion::FromAngles(RandomFloatInRange(-m_Pitch, m_Pitch), RandomFloatInRange(-m_Yaw, m_Yaw), RandomFloatInRange(-m_Roll, m_Roll));
		float s = RandomFloatInRange(m_Scale.x, m_Scale.y);
		t.scale = XMVectorSet(s, s, s, 1.f);
	}
}
