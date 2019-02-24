#pragma once
#include "stdafx.h"
#include "Component.h"

struct Vertex;

struct Mesh;

struct ConstantBuffer;
class Renderer;

#define MAX_INSTANCES 500

struct CInstanceInfo
{
	DirectX::XMMATRIX ViewProjection;
	DirectX::XMMATRIX World[MAX_INSTANCES];
	DirectX::XMMATRIX Normal[MAX_INSTANCES];
};

class InstancedMeshComponent : public Component
{
public:
	InstancedMeshComponent();	
	void CreateInstanceBuffer(Renderer* renderer);

	virtual ~InstancedMeshComponent() override;

	virtual void Initialize(Entity* owner) override;

	inline void SetMesh(Mesh* mesh) {m_Mesh = mesh; };
	Mesh* GetMesh() { return m_Mesh; }
	void AddInstance(Vector4 pos = VectorConstants::Zero, Quaternion rot = Quaternion(), Vector4 scale = DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f));
	void AddInstance(const Transform& transform);
	std::vector<Transform>* GetTransformMatrices();
	ConstantBuffer* GetConstantBuffer() { return constantBuffer; };
private:
	ConstantBuffer* constantBuffer;
	Mesh* m_Mesh;
	std::vector<Transform> m_Transforms;
};

