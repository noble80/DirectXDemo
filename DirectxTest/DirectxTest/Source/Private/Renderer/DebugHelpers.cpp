#include "stdafx.h"
#include "Renderer\DebugHelpers.h"
#include "Renderer\Renderer.h"
#include "Renderer\GraphicsStructures.h"
#include "Renderer\ResourceManager.h"
#include <assert.h>

using namespace DirectX;

GeometryBuffer* DebugHelpers::CreateDebugSphere(Renderer* renderer, int segments)
{
	assert(renderer != nullptr);


	ResourceManager* manager = renderer->GetResourceManager();
	GeometryBuffer* sphere = manager->GetResource<GeometryBuffer>("DebugSphere");

	std::vector<Vertex> vertices;

	if(sphere)
		sphere->Release();
	else
		sphere = manager->CreateResource<GeometryBuffer>("DebugSphere");

	for(int i = 0; i < segments; i++)
	{
		float phi = M_PI * (i + 1) / segments;
		for(int j = 0; j < segments; j++)
		{
			Vertex v;
			float theta = 2.0f * M_PI * j / segments;
			Vector4 pos = MathLibrary::SphericalToCartesian(theta, phi)/4.f;
			XMStoreFloat3(&v.Pos, pos);
			vertices.push_back(v);
		}
	}


	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex) * static_cast<UINT>(vertices.size());
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData{};
	InitData.pSysMem = vertices.data();

	sphere->vertexBuffer.size = static_cast<uint32_t>(vertices.size());
	sphere->vertexBuffer.data = renderer->CreateD3DBuffer(&bd, &InitData);

	DebugSphere = sphere;

	return sphere;
}

Material* DebugHelpers::DebugMat = nullptr;
GeometryBuffer* DebugHelpers::DebugSphere = nullptr;