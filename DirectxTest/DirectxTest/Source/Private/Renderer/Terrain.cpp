#include "stdafx.h"
#include "Renderer/Terrain.h"
#include "Renderer\Renderer.h"
#include "Renderer\ResourceManager.h"
#include "Renderer\Texture2D.h"
#include <d3d11_1.h>

Terrain::~Terrain()
{
}

void Terrain::Release()
{
	if(vertexBuffer) vertexBuffer->Release();
	if(indexBuffer) indexBuffer->Release();
}

void Terrain::Initialize(Renderer* renderer, std::string heightMapName, DirectX::XMFLOAT3 _scale)
{
	scale = _scale;
	constantBuffer = renderer->CreateConstantBuffer(sizeof(TerrainBuffer), heightMapName);
	TerrainBuffer* buff = static_cast<TerrainBuffer*>(constantBuffer->cpu);
		
	macroColor = renderer->LoadTexture(heightMapName + "_CM");
	macroNormal = renderer->LoadTexture(heightMapName + "_NM");
	heightmap = renderer->LoadTexture(heightMapName + "_HM");

	detailColorA = renderer->LoadTexture("Snow_CM");
	detailNormalA = renderer->LoadTexture("Snow_NM");
	detailColorB = renderer->LoadTexture("Rock_CM");
	detailNormalB = renderer->LoadTexture("Rock_NM");

	terrainMask = renderer->LoadTexture("Mountain_Mask");

	irradiance = renderer->LoadTexture("IBLTestDiffuseHDR");
	specularRef = renderer->LoadTexture("IBLTestSpecularHDR");
	integration = renderer->LoadTexture("IBLTestBrdf");

	ID3D11Texture2D* d3dtex = static_cast<ID3D11Texture2D*>(heightmap->texture);

	D3D11_TEXTURE2D_DESC desc;
	d3dtex->GetDesc(&desc);
	int texwidth = desc.Width;
	int texheight = desc.Height;

	patchVertRows = ((texwidth - 1) / patchCells) + 1;
	patchVertCols = ((texheight - 1) / patchCells) + 1;
	patchQuadCount = (patchVertRows - 1)*(patchVertCols - 1);
	patchVertCount = patchVertRows * patchVertCols;
	buff->_PatchSpacing = scale.x/(patchVertRows - 1);
	buff->_TessellationMaxDistance = 3000.f;
	buff->_TessellationMinDistance = 100.f;
	buff->_TessellationMinFactor = 2.f;
	buff->_TessellationMaxFactor = 32.f;
	buff->_TxlSizeU = 1.f / texwidth;
	buff->_TxlSizeV = 1.f / texheight;
	buff->_TextureTiling.x = 64.f;
	buff->_TextureTiling.y = 64.f;

	CreateVertexBuffer(renderer);
	CreateIndexBuffer(renderer);
}

void Terrain::CreateVertexBuffer(Renderer* renderer)
{
	TerrainVertex* patchVerts = new TerrainVertex[patchVertCount];
	float midX = scale.x / 2;
	float midZ = scale.z / 2;

	float patchX = scale.x / (patchVertCols - 1);
	float patchZ = scale.z / (patchVertRows - 1);
	float du = 1.0f / (patchVertCols - 1);
	float dv = 1.0f / (patchVertRows - 1);

	for(int i = 0; i < patchVertRows; ++i)
	{
		float z = midZ - i * patchZ;
		for(int j = 0; j < patchVertCols; ++j)
		{
			float x = -midX + j * patchX;
			int vertId = i * patchVertCols + j;

			TerrainVertex vert;
			vert.pos = {x, 0, z};
			vert.tex = {j*du, i*dv};

			patchVerts[vertId] = vert;
		}
	}

	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(TerrainVertex) * patchVertCount;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData{};
	InitData.pSysMem = patchVerts;
	vertexBuffer = renderer->CreateD3DBuffer(&bd, &InitData);

	delete[] patchVerts;
}

void Terrain::CreateIndexBuffer(Renderer* renderer)
{
	uint32_t* indices = new uint32_t[patchVertRows*patchVertCols * 4];
	int n = 0;
	for(int i = 0; i < patchVertRows - 1; ++i)
	{
		for(int j = 0; j < patchVertCols - 1; ++j)
		{
			indices[n] = i * patchVertCols + j;
			indices[n + 1] = i * patchVertCols + j + 1;
			indices[n + 2] = (i + 1) * patchVertCols + j + 1;
			indices[n + 3] = (i + 1) * patchVertCols + j;
			n += 4;
		}
	}
	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(uint32_t) * patchVertRows*patchVertCols * 4;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData{};
	InitData.pSysMem = indices;
	indexBuffer = renderer->CreateD3DBuffer(&bd, &InitData);

	delete[] indices;
}
