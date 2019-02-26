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
};

class InstancedMeshComponent : public Component
{
public:
	InstancedMeshComponent();	
	void CreateInstanceBuffer(Renderer* renderer);

	virtual ~InstancedMeshComponent() override;

	virtual void Initialize(Entity* owner) override;

	inline void SetTransform(Transform t) { m_MainTransform = t; };
	inline Transform GetTransform() { return m_MainTransform; };

	inline void SetMesh(Mesh* mesh) {m_Mesh = mesh; };
	Mesh* GetMesh() { return m_Mesh; }
	void AddInstance(Vector4 pos = VectorConstants::Zero, Quaternion rot = Quaternion(), Vector4 scale = DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f));
	void AddInstance(const Transform& transform);
	std::vector<Transform>* GetTransformMatrices();
	std::vector<Vector4> GetPositions();
	ConstantBuffer* GetConstantBuffer() { return constantBuffer; };

	void SetInstanceCount(int n);
	void RandomizeInstanceTransforms(float radius, float pitch, float yaw, float roll, Vector2 scale);
	void RandomizeInstanceTransforms();
private:

	float m_Radius;
	float m_Pitch;
	float m_Yaw;
	float m_Roll;
	Vector2 m_Scale;

	ConstantBuffer* constantBuffer;
	Mesh* m_Mesh;
	std::vector<Transform> m_Transforms;
	Transform m_MainTransform;
};

