#include "stdafx.h"
#include "Engine\InstancedMeshComponent.h"
#include "Engine/TransformComponent.h"
#include "Engine/Entity.h"

#include "Renderer\Renderer.h"

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
