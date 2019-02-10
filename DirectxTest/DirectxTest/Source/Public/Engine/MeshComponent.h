#pragma once
#include "stdafx.h"
#include "Component.h"

struct Vertex;

struct Mesh;

class TransformComponent;

class MeshComponent : public Component
{
public:

	virtual ~MeshComponent() override;

	virtual void Initialize(Entity* owner) override;

	inline void AddMesh(Mesh* mesh) { m_Meshes.push_back(mesh); };
	inline std::vector<Mesh*>& GetMeshes() { return m_Meshes; }
	DirectX::XMMATRIX GetTransformMatrix();
private:
	std::vector<Mesh*> m_Meshes;
	TransformComponent* m_Transform;
};

