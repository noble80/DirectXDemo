#pragma once

#include "GraphicsStructures.h"

struct Texture2D;

class Renderer;

struct ID3D11Buffer;

struct VertexShader;
struct PixelShader;
struct HullShader;
struct DomainShader;

struct TerrainVertex
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 tex;
};

struct TerrainBuffer
{
	float _TessellationMinDistance;
	float _TessellationMaxDistance;


	// In 2^X increments
	float _TessellationMinFactor;
	float _TessellationMaxFactor;

	DirectX::XMFLOAT2 _TextureTiling;
	float _TxlSizeU;
	float _TxlSizeV;
	float _PatchSpacing;
	DirectX::XMFLOAT3 padding;
};


struct Terrain : public Resource
{
	// Inherited via Resource
	virtual ~Terrain() override;
	virtual void Release() override;

	void Initialize(Renderer* renderer, std::string heightMapName, DirectX::XMFLOAT3 _scale);


	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;

	VertexShader* vs;
	HullShader* hs;
	DomainShader* ds;
	PixelShader* ps;
	ConstantBuffer* constantBuffer;
	int patchQuadCount;
	Texture2D* heightmap;

	Texture2D* macroColor;
	Texture2D* macroNormal;
	Texture2D* detailColorA;
	Texture2D* detailColorB;
	Texture2D* detailNormalA;
	Texture2D* detailNormalB;
	Texture2D* terrainMask;

	Texture2D* integration;
	Texture2D* irradiance;
	Texture2D* specularRef;
private:
	void CreateVertexBuffer(Renderer* renderer);
	void CreateIndexBuffer(Renderer* renderer);
	DirectX::XMFLOAT3 scale;
	int patchCells = 64;
	float patchSpacing;
	int patchVertCount;
	int patchVertRows;
	int patchVertCols;
	int width;
	int height;
};